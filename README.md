Implemented using a circular/ring buffer because then it made sense to
me what was going on. It should be possible to illustrate this quite
nicely!

```

[ ] - is used to mark the possible min elements

Push(1)

|  1
| [1]


Push(3)

| 1  3
| 1 [1]

Push(2)
Push(4)

| 1 3 2  4
| 1 1 1 [1]

Pop()

 3  2 4 |
[2] 2 4 |

Push(1)

 3  2 4 |  1
[2] 2 4 | [1]

...

```


Not as general as the other versions (yet).

[HTTP://orlp.net/blog/two-stack-sliding-window-aggregation/]
[https://stackoverflow.com/questions/4802038/implement-a-queue-in-which-push-rear-pop-front-and-get-min-are-all-consta/4802260#4802260]
[https://www.cs.cmu.edu/afs/cs.cmu.edu/academic/class/15750-s03/www/amortized-analysis.txt]
[https://hirzels.com/martin/papers/vldb15-chrprop-agg.pdf]


``` shell
Testing with window size = 49
Add and remove one element
Fill the buffer
EVAL_POP_PUSH cycle
Push: 73.4
Pop:  73.0
```

If I'm reading table2 correct in [^1] then this is faster.

[^1]:
    [DOI:10.1145/3093742.3093925](https://dl.acm.org/doi/epdf/10.1145/3093742.3093925)
