#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "gagg.h"

typedef uint64_t u64;
typedef uint32_t u32;

struct gagg_struct {
    void * Hacc; // aggregate/accumulator for the head elements
    char * RB; // Ring buffer with all elements
    uint64_t nRB; // number of element in RB (1+w)
    uint64_t H; // head = Writepos
    uint64_t U; // update horizon, or first location of the lifted elements
    uint64_t T; // tail = Readpos
    u64 mon_size;
    void * unit;
    gagg_lift_fcn lift;
    gagg_lower_fcn lower;
    gagg_combine_fcn combine;
    gagg_print_fcn print;
};

// For navigation around the boundaries
static u64 previous(const gagg * A, u64 pos){
    if(pos == 0){
        return A->nRB-1;
    }
    return pos - 1;
}

static u64 next(const gagg * A, u64 pos){
    if(pos + 1 == A->nRB){
        return 0;
    }
    return pos + 1;
}

gagg * gagg_new(uint64_t w,
               uint64_t mon_size,
               void * gagg_unit,
               gagg_lift_fcn lift,
               gagg_lower_fcn lower,
               gagg_combine_fcn combine,
               gagg_print_fcn print){
    gagg * A = calloc(1, sizeof(gagg));
    A->nRB = w+1;
    A->RB = calloc(A->nRB, mon_size);
    A->mon_size = mon_size;
    A->unit = malloc(mon_size);
    memcpy(A->unit, gagg_unit, mon_size);
    A->Hacc = gagg_unit;
    A->lift = lift;
    A->lower = lower;
    A->combine = combine;
    A->print = print;
    return A;
}

void gagg_free(gagg * A){
    if(A == NULL){
        return;
    }
    free(A->RB);
    free(A->unit);
    free(A);
    return;
}

int gagg_push(gagg * A, void * val){
    assert(A != NULL);
    assert(A->RB != NULL);

    if(next(A, A->H) == A->T){ // Full
        // Grow or crash would also make sense.
        return -1;
    }
    A->lift(A->RB + A->H*A->mon_size, val);
    A->combine(A->Hacc, A->Hacc, A->RB + A->H*A->mon_size);
    A->H = next(A, A->H);
    return 0;
}

int gagg_pop(gagg * A){
    if(A->H == A->T){ // Empty
        return -1;
    }

    int rebuild = 0;
    if(A->T == A->U){
        rebuild = 1;
    }
    A->T = next(A, A->T); // discard last element
    if(A->H == A->T){ // empty now?
        memcpy(A->Hacc, A->unit, A->mon_size);
        return 0;
    }

    if(rebuild){
        void * acc = A->RB + A->H*A->mon_size;
        memcpy(acc, A->unit, A->mon_size);
        u64 idx = previous(A, A->H);
        u64 stop_idx = previous(A, A->T);
        while(idx != stop_idx){
            A->combine(acc, acc, (void*) A->RB + idx*A->mon_size);
            memcpy(A->RB + idx*A->mon_size, acc, A->mon_size);
            idx = previous(A, idx);
        }
        memcpy(A->Hacc, A->unit, A->mon_size);
        A->U = A->H;
    }
    return 0;
}

int gagg_eval(const gagg * A, void  *val){
    if(A->T == A->H){ // completely empty
        A->lower(val, A->unit);
        return -1;
    }
    if(A->T == A->U){  // empty tail
        A->lower(val, A->Hacc); // in the head we trust
        return 0;
    }
    // combine to A->H which is unused.
    A->combine( (void*) A->RB + A->mon_size*A->H,
             A->Hacc,
             (void*) A->RB + A->T*A->mon_size);
    A->lower(val, A->RB + A->H*A->mon_size);
    return 0;
}

// Print by insertion order,
// i.e. tail element first and head element last
void gagg_print(const gagg * A){
    assert(A != NULL);
    u64 pos = A->T;
    u64 n_elements = 0;
    int state = 0; // AGGT()s
    printf("---\n");
    while(pos != A->H){
        if(pos == A->U){
            state = 1;
        }
        printf("| %lu ", pos);;
        if(state == 0){
            printf("AGG=");
        } else {
            printf("EL =");
        }
        A->print(A->RB + pos*A->mon_size);
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
    A->print(A->Hacc);
    if(memcmp(&A->Hacc, &A->unit, A->mon_size) == 0){
        printf("(=mono0) ");
    }
    printf("\n");
    printf("---\n");
    return;
}
