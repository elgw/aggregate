#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "aggregate.h"

typedef uint64_t u64;

typedef struct {
    double val;
    double agg;
} agg;

// combine, lift, lower
const agg agg0 = {.val = INFINITY, .agg = INFINITY};
const agg agg1 = {.val = INFINITY, .agg = INFINITY};
static agg lift(double v){
    return (agg) {.val = v, .agg = v};
    //agg r = {v, v}; return r;
}

static double lower(agg a){
    return a.val;
}

static agg combine(agg a, agg b){
    if(a.agg < b.agg){
        return a;
    }
    return b;
}

// Algorithm below

struct aggregate_struct {
    agg * pairs;
    uint64_t n, capacity;
    uint64_t H; // head = Writepos
    agg aggH; // aggregate of the head elements
    uint64_t T; // tail = Readpos
    uint64_t U; // update horizon
};


static u64 previous(const aggregate * A, u64 pos){
    if(pos == 0){
        return A->capacity-1;
    }
    return pos - 1;
}

static u64 next(const aggregate * A, u64 pos){
    if(pos + 1 == A->capacity){
        return 0;
    }
    return pos + 1;
}

aggregate * aggregate_new(uint64_t max_window_size){
    aggregate * A = calloc(1, sizeof(aggregate));
    A->capacity = max_window_size+1;
    A->pairs = calloc(A->capacity, sizeof(agg));
    A->aggH = agg0;
    return A;
}

void aggregate_free(aggregate * A){
    if(A == NULL){
        return;
    }
    free(A->pairs);
    free(A);
    return;
}

int aggregate_push(aggregate * A, double val){
    //printf("Pushing %f\n", val);
    assert(A != NULL);
    assert(A->pairs != NULL);

    // Since the buffer has a fixed size, it can happen that
    // it is full. We might want to crash here...
    if(next(A, A->H) == A->T){
        return -1;
    }
    A->pairs[A->H] = lift(val);
    A->aggH = combine(A->aggH, A->pairs[A->H]);
    A->H = next(A, A->H);
    return 0;
}

int aggregate_pop(aggregate * A){
    // The buffer might be empty
    if(A->H == A->T){
        return -1;
    }

    int rebuild = 0;
    if(A->T == A->U){
        rebuild = 1;
    }
    A->T = next(A, A->T); // discard last element
    if(A->H == A->T){ // empty now?
        A->aggH = agg0;
        return 0;
    }

    if(rebuild){
        agg a = agg1;
        u64 idx = previous(A, A->H);
        u64 stop_idx = previous(A, A->T);
        while(idx != stop_idx){
            a = combine(a, A->pairs[idx]);
            A->pairs[idx] = a;
            idx = previous(A, idx);
        }
        A->aggH = agg1;
        A->U = A->H;
    }

    return 0;
}

int aggregate_eval(const aggregate * A, double  *val){
    // TODO: this should be when the tail is empty
    // how can we know if the tail is empty?
    if(A->T == A->U){  // empty
        *val = lower(A->aggH);
    }
    *val = lower(combine(A->aggH, A->pairs[A->T]));
    return 0;
}

void aggregate_reset(aggregate * A){
    A->H = 0;
    A->T = 0;
    A->U = 0;
    #ifndef NDEBUG
    memset(A->pairs, 0, A->capacity*sizeof(agg));
    #endif
    return;
}

// Print by insertion order,
// i.e. tail element first and head element last
void aggregate_print(const aggregate * A){
    u64 pos = A->T;
    double minval = INFINITY;
    u64 n_elements = 0;
    while(pos != A->H){
        if(minval > A->pairs[A->T].val){
            minval = A->pairs[A->T].val;
        }
        printf("%lu (%.1f, %.1f)", pos,
               A->pairs[pos].val,
               A->pairs[pos].agg);
        if(A->pairs[pos].val < minval){
            minval = A->pairs[pos].val;
        }
        if(A->T == pos){
            printf("T");
        }
        if(A->H == pos){
            printf("H");
        }
        if(A->U == pos){
            printf("U");
        }
        printf("\n");
        pos = next(A, pos);
        n_elements++;
    }
    printf("aggH: (%f, %f)\n", A->aggH.val, A->aggH.agg);
    printf("---\n");
    double a_minval;
    aggregate_eval(A, &a_minval);
    if(n_elements > 0){
        if(a_minval != minval){
            printf("Minval found in the data: %f\n", minval);
            printf("Minval reported by agg:   %f\n", a_minval);
            assert(0);
        }
    }
}
