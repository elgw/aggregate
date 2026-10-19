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

# - "empty" not needed, i.e. the unit in the monoid
#   unit/"lift" called directly
#   at the cost of a few more comparisons
# - 1 less combine operation in pop

class TwoStackAgg2:
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
            self.cum_aggs.append(cum_agg)
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

# A rolling buffer
class RollingAgg:
    def __init__(self, w):
        self.values = [0]*(w+1)
        self.cap = w+1
        self.agg = None
        self.H = 0
        self.T = 0
        self.U = 0

    def next(self, i):
        return i + 1 if i + 1 < self.cap else 0

    def prev(self, i):
        return i -1 if i > 0 else self.cap -1

    def push(self, x):
        self.values[self.H] = unit(x)
        if self.agg is None:
            self.agg = self.values[self.H]
        else:
            self.agg = combine(self.agg, self.values[self.H])
        self.H = self.next(self.H)

    def pop(self):
        if(self.H == self.T):
            assert(0) # failure -- pop on empty
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


n_test = 1000000
w = 2**14
T1 = TwoStackAgg() # 962 ns
#T2 = TwoStackAgg2() # 912 ns
T2 = RollingAgg(w) # 1262 ns
t_T1 = 0
t_T2 = 0

# FILL
for i in range(0, w):
    T1.push(i)
    T2.push(i)
    assert(T1.eval() == T2.eval())

# POP PUSH EVAL
for i in range(0, n_test):
    v = random.randint(1, 100)
    e1 = 0
    e2 = 0

    t0 = time.time_ns()
    T1.pop(),
    T1.push(v)
    e1 = T1.eval()
    t1 = time.time_ns()
    t_T1 += t1-t0

    t0 = time.time_ns()
    T2.pop()
    T2.push(v)
    e2 = T2.eval()
    t1 = time.time_ns()
    t_T2 += t1-t0
    assert(e1 == e2)

# EMPTY
# eval on empty skipped, different return values
for i in range(0, w-1):
    T1.pop()
    T2.pop()
    assert(T1.eval() == T2.eval())

print('Average time [ns]')
print(f'T1: {t_T1/n_test}')
print(f'T2: {t_T2/n_test}')
