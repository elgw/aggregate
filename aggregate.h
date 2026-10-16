#pragma once
#include <stdint.h>

typedef enum {
    aggregate_min
} aggregate_type;

typedef struct {
    double value;
    double aggregate;
} aggregate_pair;

typedef struct {
    aggregate_pair * pairs;
    uint64_t n, capacity;
    uint64_t H; // head = Writepos
    uint64_t T; // tail = Readpos
    uint64_t U; // update horizon
} aggregate;

aggregate * aggregate_new(uint64_t max_window_size, aggregate_type t);
void aggregate_free(aggregate * );
int aggregate_push(aggregate * A, double value);
int aggregate_pop(aggregate * A);
int aggregate_eval(const aggregate * A, double  * value);

void aggregate_reset(aggregate * A);
void aggregate_print(const aggregate * A);
