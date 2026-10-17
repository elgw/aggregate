#  Sliding-Window Aggregation with Ring Buffer

Context: Sliding-Window Aggregation, described in [^1] and [^2] see
also [^3]. The background will not be repeated here.

Observation: For a window of $`w`$ elements, the two-stack approach
maintains two lists of size $`w`$, one for elements and one for
aggregates. That seems wasteful since these two lists contain at most
$`w`$ elements in total.

Tested here: A single list, implemented as a circular/ring buffer

## Results

Quick tests shows reasonable performance. At the moment there are
timings for my code run on my machine, vs that of [^2], evaluated on
their system.

The table will show how many ticks it takes (`__rdtscp`) in average
for each cycle of **pop**, **push** and **eval** using a window size
of $`2^{14}`$ elements.

Two-Stacks result taken from Table 2 in [^2]. CPU: Intel Xeon E5-2680
at 2.7 GHz,  g++ version 4.8.3.

Timings for this code: AMD Ryzen 3700X, gcc version 13.3.0.

``` shell
make -B OPT=1 test_min -B  && ./test_min
make -B OPT=1 test_mean -B && ./test_mean
make -B OPT=1 test_min -B  && ./test_stddev
```

|           | Two-Stacks | This |
|----------:|-----------:|-----:|
|   Max/Min |        125 |   77 |
| ArithMean |        165 |   79 |
|    StdDev |        200 |   95 |


## Implementation / Memory Layout

Unused space shown as `____`, an accumulator is denote `MAC` and an
element, barely lifted is denoted `LEL`.

With 5 elements in the data structures of capacity 7; 3 at the "Front"
and 2 in the "Back", the layout could look like this:

```
Hacc = [MAC]
         |
        Accumulator for the head elements marked below
                          >--------<
RB  = [___ ,MAC, MAC, MAC, LEL, LEL), ___]
             |              |         |
            (T)ail          |        (H)head
                           (U)pdate Horizon /
                           First "normal" element
```

A call to `accumulate_eval` will return

## To do ...

- [ ] Check that it is correct.
- [ ] Make it useful; a little, or some more macros could help (so
      much nicer to do it a language with templates or metaprogramming
      ... ).
- [ ] Find a use for it :)

## References

[^1]: Kanat Tangwongsan, Martin Hirzel, Scott Schneider, and Kun-Lung Wu. 2015. General incremental sliding-window aggregation. Proc. VLDB Endow. 8, 7 (February 2015), 702–713. https://doi.org/10.14778/2752939.2752940
[^2]: Kanat Tangwongsan, Martin Hirzel, and Scott Schneider. 2017. Low-Latency Sliding-Window Aggregation in Worst-Case Constant Time. In Proceedings of the 11th ACM International Conference on Distributed and Event-based Systems (DEBS '17). Association for Computing Machinery, New York, NY, USA, 66–77. https://doi.org/10.1145/3093742.3093925
[^3]: Orson R. L. Peters, Two-Stack Sliding-Window Aggregation,
    [https://orlp.net/blog/two-stack-sliding-window-aggregation/]
