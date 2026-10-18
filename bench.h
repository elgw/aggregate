#pragma once

// Benchmark for a fixed window size and a given time budget
void agg_min_bench(int verbose, int w, double timeout);
void agg_mean_bench(int verbose, int w, double timeout);
void agg_std_bench(int verbose, int w, double timeout);

// Show steps as the aggregator is manipulated
void agg_min_steps(int w);
void agg_mean_steps(int w);
void agg_std_steps(int w);

// Unit tests
void agg_min_ut(int verbose, int w);
void agg_mean_ut(int verbose, int w);
void agg_std_ut(int verbose, int w);
