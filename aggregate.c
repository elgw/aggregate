#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "aggregate.h"

typedef uint64_t u64;
typedef uint32_t u32;

#ifdef AGGREGATE_MIN

typedef struct {
    double val;
} el;

// Aggregator for Min value, only one value needed
typedef struct {
    double a;
} agg;

// combine, lift, lower
const agg agg0 = {.a = INFINITY};
const agg agg1 = {.a = INFINITY};

static agg lift(double v){
    return (agg) {.a = v};
}

static double lower(agg a){
    return a.a;
}

static agg combine(agg a, agg b){
    if(a.a < b.a){
        return a;
    }
    return b;
}

static void agg_print(agg a){
    printf("[.a=%f]", a.a);
}
#endif

#ifdef AGGREGATE_MEAN

typedef struct {
    double val;
} el;

typedef struct {
    u32 n;    // number of elements
    double S; // Sum
} agg;

// combine, lift, lower
const agg agg0 = {.n = 0, .S = 0};
const agg agg1 = {.n = 0, .S = 0};

static agg lift(double v){
    return (agg) {.n = 1, .S = v};
}

static double lower(agg a){
    return a.S / a.n;
}

static agg combine(agg a, agg b){
    return (agg) {.n=a.n + b.n, .S = a.S + b.S};
}

static void agg_print(agg a){
    printf("[.n=%u, .S=%f]", a.n, a.S);
}
#endif

#ifdef AGGREGATE_STDDEV

typedef struct {
    double val;
} el;

typedef struct {
    u32 n;    // number of elements
    double S; // Sum
    double Q; // Sum of Squares
} agg;

// combine, lift, lower
const agg agg0 = {.n = 0, .S = 0, .Q = 0};
const agg agg1 = {.n = 0, .S = 0, .Q = 0};

static agg lift(double v){
    return (agg) {.n = 1, .S = v, .Q = v*v};
}

static double lower(agg a){
    if(a.n == 0){
        return 0.0;
    }
    return sqrt( 1.0 / (double) a.n * (a.Q - a.S*a.S/(double) a.n));
}

static agg combine(agg a, agg b){
    return (agg) {.n=  a.n + b.n,
                  .S = a.S + b.S,
                  .Q = a.Q + b.Q};
}

static void agg_print(agg a){
    printf("[.n=%u, .S=%f .Q=%f]", a.n, a.S, a.Q);
}
#endif

// Algorithm below

typedef union {
    agg agg;
    el el;
} item;

struct aggregate_struct {
    item* RB; // Ring buffer
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
    A->RB = calloc(A->capacity, sizeof(agg));
    A->aggH = agg0;
    return A;
}

void aggregate_free(aggregate * A){
    if(A == NULL){
        return;
    }
    free(A->RB);
    free(A);
    return;
}

int aggregate_push(aggregate * A, double val){
    //printf("Pushing %f\n", val);
    assert(A != NULL);
    assert(A->RB != NULL);

    // Since the buffer has a fixed size, it can happen that
    // it is full. We might want to crash here...
    if(next(A, A->H) == A->T){
        return -1;
    }
    A->RB[A->H].agg = lift(val);
    A->aggH = combine(A->aggH, A->RB[A->H].agg);
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
            a = combine(a, A->RB[idx].agg);
            A->RB[idx].agg = a;
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
        return 0;
    }
    *val = lower(combine(A->aggH, A->RB[A->T].agg));
    return 0;
}

void aggregate_reset(aggregate * A){
    A->H = 0;
    A->T = 0;
    A->U = 0;
    #ifndef NDEBUG
    memset(A->RB, 0, A->capacity*sizeof(agg));
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
        if(minval > A->RB[A->T].el.val){
            minval = A->RB[A->T].el.val;
        }
        printf("%lu ", pos);;
        agg_print(A->RB[pos].agg);
        if(A->RB[pos].el.val < minval){
            minval = A->RB[pos].el.val;
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
    printf("aggH: ");
    agg_print(A->aggH);
    printf("\n");
    printf("---\n");
}
