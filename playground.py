import random
import time

# fn empty() -> Agg;
# fn unit(x: Value) -> Agg;            // lift
# fn combine(x: Agg, y: Agg) -> Agg;
# fn finalize(x: Agg) -> Out;          // lower

if 1: # mean value
    empty = lambda: (0, 0)
    unit = lambda x: (x, 1) # lift
    combine = lambda x, y: (x[0] + y[0], x[1] + y[1])
    finalize = lambda x: x[0] / x[1] if x[1] else None # lower

if 0: # min type
    empty = lambda: (1e99, 1e99)
    unit = lambda x: (x, x)
    combine = lambda x, y: (min(x[0],y[0]), min(x[1],y[1]))
    finalize = lambda x: x[0]

# From https://orlp.net/blog/two-stack-sliding-window-aggregation/
class TwoStackAgg:
    def __init__(self):
        self.values = []
        self.values_agg = empty()
        self.cum_aggs = []

    def push(self, x):
        self.values.append(x)
        self.values_agg = combine(self.values_agg, unit(x))

    def pop(self):
        if not self.cum_aggs:
            cum_agg = empty()
            while self.values:
                cum_agg = combine(unit(self.values.pop()), cum_agg)
                self.cum_aggs.append(cum_agg)
            self.values_agg = empty()
        self.cum_aggs.pop()

    def eval(self):
        return finalize(
            combine(self.cum_aggs[-1], self.values_agg)
            if self.cum_aggs else self.values_agg
        )


class TwoStackAgg2:
    """
    - semigroup aggregate, i.e. identity/"empty" not needed. Seems to be possible
    at the cost of a few more conditionals. Also changes the eval of an empty
    structure
    - 1 less combine operation in pop during the rebuild.
    """

    def __init__(self):
        self.values = []
        self.agg = None
        self.cum_aggs = []

    def push(self, x):
        self.values.append(unit(x))
        if self.agg is None:
            self.agg = self.values[-1]
        else:
            self.agg = combine(self.agg, self.values[-1])

    def pop(self):
        if not self.cum_aggs:
            cum_agg = self.values.pop()
            self.cum_aggs.append(cum_agg) # Note: not combined's with the identity
            while self.values:
                cum_agg = combine(self.values.pop(), cum_agg)
                self.cum_aggs.append(cum_agg)
            self.agg = None
        self.cum_aggs.pop()

    def eval(self):
        if self.agg is None:
            if len(self.cum_aggs) == 0:
                return None
            return finalize(self.cum_aggs[-1])
        if len(self.cum_aggs) == 0:
            return finalize(self.agg)
        return finalize(combine(self.cum_aggs[-1], self.agg))


class CircAgg:
    """
    Using a circular buffer to store the lifted elements. Only (w+1) elements needed,
    however some extra book keeping with cap, H, T, and U.

    Not so efficient in Python given that lists element contain pointers to objects ...

    Would be more elegant if we used 1-element, then agg would never have to be None

    Constructed with (windows size)
    """
    def __init__(self, w):
        self.values = [0]*(w+1) # Not a true array
        self.cap = w+1 # capacity
        self.agg = None # Aggregator for elements
        self.H = 0 # Where to push the next element
        self.T = 0 # Oldest element
        self.U = 0 # Pos of first item in agg

    def next(self, i):
        return i + 1 if i + 1 < self.cap else 0

    def prev(self, i):
        return i -1 if i > 0 else self.cap -1

    def push(self, x):
        assert(self.next(self.H) != self.T)
        self.values[self.H] = unit(x)
        if self.agg is None:
            self.agg = self.values[self.H]
        else:
            self.agg = combine(self.agg, self.values[self.H])
        self.H = self.next(self.H)

    def pop(self):
        assert(self.H != self.T) # Can't pop when empty

        rebuild = 0
        if self.T == self.U:
            rebuild = 1

        self.values[self.T] = None
        self.T = self.next(self.T)

        if self.H == self.T: # empty now
            self.agg = None
            return

        if rebuild:
            idx = self.prev(self.H)
            stop_idx = self.prev(self.T)
            A = self.values[idx]
            idx = self.prev(idx)
            while idx != stop_idx:
                A = combine(A, self.values[idx])
                self.values[idx] = A
                idx = self.prev(idx)
            self.agg = None
            self.U = self.H

    def eval(self):
        if self.T == self.H:
            return None
        if self.T == self.U:
            return finalize(self.agg)
        if self.agg is None:
            return finalize(self.values[self.T])
        return finalize(combine(self.agg, self.values[self.T]))

    def print(self):
        C = [0]*self.cap
        S = []
        for e in self.values: S.append(f'{e} ')
        M = ['' for s in S]
        M[self.U] = M[self.U] + 'U'
        M[self.T] = M[self.T] + 'T'
        M[self.H] = M[self.H] + 'H'
        M2 = [ m + ' '*(len(s)-len(m)) for s, m in zip(S, M)]
        for s in S: print(f'{s} ', end='')
        print('')
        for m in M2: print(f'{m} ', end='')
        print('')
        print(f'AGG = {self.agg}')
        print(f'eval = {self.eval()}')


n_test = 100000
w = 2**14

T1 = TwoStackAgg() # 962 ns
T2 = TwoStackAgg2() # 912 ns
T3 = CircAgg(w) # 1262 ns

TT = [ T1, T2, T3 ]
timings = [0]*len(TT)

# FILL
for i in range(0, w):
    for T in TT:
        T.push(i)

    v = TT[0].eval()
    for T in TT[1:]:
        assert(v == T.eval())

# POP PUSH EVAL
for i in range(0, n_test):
    v = random.randint(1, 100)
    e1 = 0
    e2 = 0
    for i, T in enumerate(TT):
        t0 = time.time_ns()
        T.pop(),
        T.push(v)
        e1 = T1.eval()
        t1 = time.time_ns()
        timings[i] += t1-t0

    v = TT[0].eval()
    for T in TT[1:]:
        assert(v == T.eval())

# EMPTY
# eval on empty skipped, different return values
for i in range(0, w-1):
    for T in TT:
        T.pop()
    v = TT[0].eval()
    for T in TT[1:]:
        assert(v == T.eval())

print('Average time [ns]')
for k, T in enumerate(TT):
    print(f'{T.__class__} : {timings[k]/n_test}')
