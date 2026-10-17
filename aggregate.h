#pragma once
#include <stdint.h>

struct aggregate_struct;
typedef struct aggregate_struct aggregate;

aggregate * aggregate_new(uint64_t max_window_size);
void aggregate_free(aggregate * );
int aggregate_push(aggregate * A, double value);
int aggregate_pop(aggregate * A);
int aggregate_eval(const aggregate * A, double  * value);

void aggregate_reset(aggregate * A);
void aggregate_print(const aggregate * A);
