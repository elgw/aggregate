#pragma once
#include <stdint.h>

// The interface looks like a FIFO-queue with these methods:
//
// - **push** -- add to the accumulator. Use one combine operation.
// - **eval** -- get the current estimate. Requires one combine operation.
// - **pop** -- remove the first inserted element. Uses in average O(1) operations.
//
// What is different here is that the eval operation estimates whatever measurement
// the code was compiled with.

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
