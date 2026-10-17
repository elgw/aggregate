#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "aggregate.h"

typedef uint64_t u64;
typedef uint32_t u32;

// There is one #define guarded section per aggregation type below
//
// mitem -- a lifted element, i.e. a monoid element/item.
// mono0 -- the 0 element in the monoid
// lift -- convert/lift from input domain to monoid domain
// lower -- convert from monoid/mitem to input domain
// combine -- combine two monoid elements
// mitem_print -- used by aggregate_print

#ifdef AGGREGATE_MIN
typedef struct {
    double a; // min value
} mitem;
const mitem mono0 = {.a = INFINITY};
static mitem lift(double v){
    return (mitem) {.a = v};
}
static double lower(mitem a){
    return a.a;
}
static mitem combine(mitem a, mitem b){
    if(a.a < b.a){
        return a;
    }
    return b;
}
static void mitem_print(mitem a){
    printf("[.a=%f]", a.a);
}
#endif

#ifdef AGGREGATE_MEAN
typedef struct {
    double val;
} el;
typedef struct {
    u64 n;    // number of elements
    double S; // Sum
} mitem;
const mitem mono0 = {.n = 0, .S = 0};
static mitem lift(double v){
    return (mitem) {.n = 1, .S = v};
}
static double lower(mitem a){
    return a.S / a.n;
}
static mitem combine(mitem a, mitem b){
    return (mitem) {.n=a.n + b.n, .S = a.S + b.S};
}
static void mitem_print(mitem a){
    printf("[.n=%lu, .S=%f]", a.n, a.S);
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
const mitem mono0 = {.n = 0, .S = 0, .Q = 0};
static mitem lift(double v){
    return (mitem) {.n = 1, .S = v, .Q = v*v};
}
static double lower(mitem a){
    if(a.n == 0){
        return 0.0;
    }
    return sqrt( 1.0 / (double) a.n * (a.Q - a.S*a.S/(double) a.n));
}
static mitem combine(mitem a, mitem b){
    return (mitem) {.n=  a.n + b.n,
        .S = a.S + b.S,
        .Q = a.Q + b.Q};
}
static void mitem_print(mitem a){
    printf("[.n=%u, .S=%f .Q=%f]", a.n, a.S, a.Q);
}
#endif

// - [x] monoid set up,
// - [ ] data structure of the algorithm
//
//  [T, .., U[ contains aggregated items
//  [U, .., H] contains items that are only lifted
//
// Use aggregate_next, and aggrepage_previous to walk the arrays in a
// Ring-Buffer way.

struct aggregate_struct {
    mitem Hacc; // aggregate/accumulator for the head elements
    mitem* RB; // Ring buffer with all elemnts
    uint64_t nRB; // number of element in RB
    uint64_t H; // head = Writepos
    uint64_t U; // update horizon, or first location of the lifted elements
    uint64_t T; // tail = Readpos
};

// For navigation around the boundaries
static u64 previous(const aggregate * A, u64 pos){
    if(pos == 0){
        return A->nRB-1;
    }
    return pos - 1;
}

static u64 next(const aggregate * A, u64 pos){
    if(pos + 1 == A->nRB){
        return 0;
    }
    return pos + 1;
}

aggregate * aggregate_new(uint64_t max_window_size){
    aggregate * A = calloc(1, sizeof(aggregate));
    A->nRB = max_window_size+1;
    A->RB = calloc(A->nRB, sizeof(mitem));
    A->Hacc = mono0;
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
    assert(A != NULL);
    assert(A->RB != NULL);

    if(next(A, A->H) == A->T){ // Full
        // Grow or crash would also make sense.
        return -1;
    }
    A->RB[A->H] = lift(val);
    A->Hacc = combine(A->Hacc, A->RB[A->H]);
    A->H = next(A, A->H);
    return 0;
}

int aggregate_pop(aggregate * A){
    if(A->H == A->T){ // Empty
        return -1;
    }

    int rebuild = 0;
    if(A->T == A->U){
        rebuild = 1;
    }
    A->T = next(A, A->T); // discard last element
    if(A->H == A->T){ // empty now?
        A->Hacc = mono0;
        return 0;
    }

    if(rebuild){
        mitem a = mono0;
        u64 idx = previous(A, A->H);
        u64 stop_idx = previous(A, A->T);
        while(idx != stop_idx){
            a = combine(a, A->RB[idx]);
            A->RB[idx] = a;
            idx = previous(A, idx);
        }
        A->Hacc = mono0;
        A->U = A->H;
    }
    return 0;
}

int aggregate_eval(const aggregate * A, double  *val){
    if(A->T == A->H){ // completely empty
        return -1;
    }
    if(A->T == A->U){  // empty tail
        *val = lower(A->Hacc); // in the head we trust
        return 0;
    }
    *val = lower(combine(A->Hacc, A->RB[A->T]));
    return 0;
}

void aggregate_reset(aggregate * A){
    A->H = 0;
    A->T = 0;
    A->U = 0;
#ifndef NDEBUG
    memset(A->RB, 0, A->nRB*sizeof(mitem));
#endif
    A->Hacc = mono0;
    return;
}

// Print by insertion order,
// i.e. tail element first and head element last
void aggregate_print(const aggregate * A){
    assert(A != NULL);
    u64 pos = A->T;
    u64 n_elements = 0;
    int state = 0; // aggregates
    printf("---\n");
    while(pos != A->H){
        if(pos == A->U){
            state = 1;
        }
        printf("| %lu ", pos);;
        if(state == 0){
            printf("AGG=");
            mitem_print(A->RB[pos]);
        } else {
            printf("EL =");
            mitem_print(A->RB[pos]);
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
    printf("| Hacc: ");
    mitem_print(A->Hacc);
    if(memcmp(&A->Hacc, &mono0, sizeof(mitem)) == 0){
        printf("(=mono0) ");
    }
    printf("\n");
    printf("---\n");
    return;
}
