#  Sliding-Window Aggregation with Ring Buffer

Warning, throwaway code ahead!

Context: Sliding-Window Aggregation, discussed in this [HN
thread](https://news.ycombinator.com/item?id=50072485)/[Orson
R. L. Peters'
blog](https://orlp.net/blog/two-stack-sliding-window-aggregation/) and
described in [^1] and [^2].

My Observation: In the two-stacks approach, it seems wasteful to
mainintain two windows of $`w`$ elements, one should be enough!

Tested here: A single list, implemented as a circular/ring buffer

## Results

Quick tests shows reasonable performance.

Two-Stacks result taken from Table 2 in [^2]. CPU: Intel Xeon E5-2680
at 2.7 GHz,  g++ version 4.8.3.

My code was timed on an AMD Ryzen 3700X, gcc version 13.3.0 using:

``` shell
$ make OPT=1 -B && ./test --benchmark --timeout 10
```

The results are **NOT** comparable, at most indicative, due to the
differences in IPC, compiler, test setups etc.

The table will show how many ticks it takes (`__rdtscp`) in average
for each cycle of **pop**, **push** and **eval** using a window size
of $`2^{14}`$ elements, (at least that is how my code is measured).

|           | Two-Stacks | This |
|----------:|-----------:|-----:|
|   Max/Min |        125 |   67 |
| ArithMean |        165 |   77 |
|    StdDev |        200 |   93 |

Throughput can be approximated by `cpufreq/latency*sizeof(u64)`
bytes per second.


## Implementation / Memory Layout

Unused space shown as `____`, barely lifted elements `LEL`, and
elements that accumulate/combine results `MA#` where `#` says how many
elements that it combines.

With 5 elements in the data structures of capacity 7; with 3 elements
at the "Front" and 2 in the "Back", the layout could look like this:

```
Hacc = [MA2]
         |
        Accumulator for the head elements marked below
                          >--------<
RB = [___ ,MA3, MA2, MA1, LEL, LEL), ___]
             |              |         |
            (T)ail          |        (H)head
                           (U)pdate Horizon /
                           First "normal" element

The output combines Tail with Hacc into a MA5 which is then lowered.
```

To see what is going on using random data, try `./test --steps`

<details><summary>./test --steps --method 2</summary>

Small excerpt ...

``` shell
Showing steps for std

new, w=5
---
| Hacc: [.n=0, .S=0.000000 .Q=0.000000](=mono0)
---
Filling ...
push(A, 0.840188)
---
| 0 EL =[.n=1, .S=0.840188 .Q=0.705915]TU
| Hacc: [.n=1, .S=0.840188 .Q=0.705915]
---
push(A, 0.394383)
---
| 0 EL =[.n=1, .S=0.840188 .Q=0.705915]TU
| 1 EL =[.n=1, .S=0.394383 .Q=0.155538]
| Hacc: [.n=2, .S=1.234571 .Q=0.861453]
---
push(A, 0.783099)
---
| 0 EL =[.n=1, .S=0.840188 .Q=0.705915]TU
| 1 EL =[.n=1, .S=0.394383 .Q=0.155538]
| 2 EL =[.n=1, .S=0.783099 .Q=0.613244]
| Hacc: [.n=3, .S=2.017670 .Q=1.474698]
---
push(A, 0.798440)
---
| 0 EL =[.n=1, .S=0.840188 .Q=0.705915]TU
| 1 EL =[.n=1, .S=0.394383 .Q=0.155538]
| 2 EL =[.n=1, .S=0.783099 .Q=0.613244]
| 3 EL =[.n=1, .S=0.798440 .Q=0.637506]
| Hacc: [.n=4, .S=2.816110 .Q=2.112204]
---
push(A, 0.911647)
---
| 0 EL =[.n=1, .S=0.840188 .Q=0.705915]TU
| 1 EL =[.n=1, .S=0.394383 .Q=0.155538]
| 2 EL =[.n=1, .S=0.783099 .Q=0.613244]
| 3 EL =[.n=1, .S=0.798440 .Q=0.637506]
| 4 EL =[.n=1, .S=0.911647 .Q=0.831101]
| Hacc: [.n=5, .S=3.727757 .Q=2.943305]
---
POP-PUSH-EVAL ...
pop(A)
---
| 1 AGG=[.n=4, .S=2.887570 .Q=2.237390]T
| 2 AGG=[.n=3, .S=2.493187 .Q=2.081852]
| 3 AGG=[.n=2, .S=1.710087 .Q=1.468607]
| 4 AGG=[.n=1, .S=0.911647 .Q=0.831101]
| Hacc: [.n=0, .S=0.000000 .Q=0.000000]
---
push(A, 0.197551)
---
| 1 AGG=[.n=4, .S=2.887570 .Q=2.237390]T
| 2 AGG=[.n=3, .S=2.493187 .Q=2.081852]
| 3 AGG=[.n=2, .S=1.710087 .Q=1.468607]
| 4 AGG=[.n=1, .S=0.911647 .Q=0.831101]
| 5 EL =[.n=1, .S=0.197551 .Q=0.039027]U
| Hacc: [.n=1, .S=0.197551 .Q=0.039027]
---
eval(A) -> 0.273065
```

</details>

## Validation

Use `make -B && ./test --validate` to run some tests. They are not
exhaustive! Assume that nothing works until you've written your own
tests.

## References/Links

[^1]: Kanat Tangwongsan, Martin Hirzel, Scott Schneider, and Kun-Lung Wu. 2015. General incremental sliding-window aggregation. Proc. VLDB Endow. 8, 7 (February 2015), 702–713. https://doi.org/10.14778/2752939.2752940
[^2]: Kanat Tangwongsan, Martin Hirzel, and Scott Schneider. 2017. Low-Latency Sliding-Window Aggregation in Worst-Case Constant Time. In Proceedings of the 11th ACM International Conference on Distributed and Event-based Systems (DEBS '17). Association for Computing Machinery, New York, NY, USA, 66–77. https://doi.org/10.1145/3093742.3093925
[^3]: Orson R. L. Peters, Two-Stack Sliding-Window Aggregation,
    [https://orlp.net/blog/two-stack-sliding-window-aggregation/]

- For large windows consider [Kahan
  summation](https://en.wikipedia.org/wiki/Kahan_summation_algorithm)
  and similar techniques depending on statistics of interest. See also
  [The GNU Multiple Precision Arithmetic Library](https://gmplib.org/).
