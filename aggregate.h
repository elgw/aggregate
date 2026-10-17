#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// The interface looks like a FIFO-queue with these methods:
//
// - **push** -- add to the accumulator. Use one combine operation.
// - **eval** -- get the current estimate. Requires one combine operation.
// - **pop** -- remove the first inserted element. Uses in average O(1) operations.
//
// What is different here is that the eval operation gives the statistics
// the code was compiled for.
//
// Can be compile for one statistics at a time, please define one of
// the following options:
//
// AGGREGATE_MIN    -- For min value
// AGGREGATE_MEAN   -- For median value
// AGGREGATE_STDDEV -- For standard deviation

// TODO
// #define aggt(X) X##_min
// #define aggt(X) X##_mean
// #define aggt(X) X##_std

struct aggregate_struct;
typedef struct aggregate_struct aggregate;

// What you would expect. It would of course be possible to
// make the structure dynamic, i.e. let it grow whenever needed.
aggregate * aggregate_new(uint64_t max_window_size);

void aggregate_free(aggregate * );

int aggregate_push(aggregate * A, double value);
int aggregate_pop(aggregate * A);
int aggregate_eval(const aggregate * A, double  * value);

// Utility methods

// Clear the contents of the aggregate, i.e. reset as if brand new
void aggregate_reset(aggregate * A);
// Print some diagnostics to the screen
void aggregate_print(const aggregate * A);

#ifdef __cplusplus
}
#endif
