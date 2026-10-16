#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "aggregate.h"

typedef uint64_t u64;


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

static double min_double(double a, double b){
    if(a < b){
        return a;
    }
    return b;
}

aggregate * aggregate_new(uint64_t max_window_size, aggregate_type t){
    if(t != aggregate_min){
        return NULL;
    }
    aggregate * A = calloc(1, sizeof(aggregate));
    A->capacity = max_window_size+1;
    A->pairs = calloc(A->capacity, sizeof(aggregate_pair));
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

int aggregate_push(aggregate * A, double value){
    assert(A != NULL);
    assert(A->pairs != NULL);

    if(next(A, A->H) == A->T){ // full
        return -1;
    }
    A->pairs[A->H].value = value;

    if(A->H != A->T){ // not empty
        value = min_double(value, A->pairs[previous(A, A->H)].aggregate);
        //printf("uvalue: %f\n", value);
    }
    A->pairs[A->H].aggregate = value;
    #if 0
    printf("NEW pos: %lu value = %f, agg = %f\n",
           A->H,
           A->pairs[A->H].value,
           A->pairs[A->H].aggregate);
    #endif
    A->H = next(A, A->H);
    return 0;
}

int aggregate_pop(aggregate * A){
    if(A->H == A->T){
        return -1;
    }
    int rebuild = 0;
    if(A->T == A->U){
        rebuild = 1;
    }
    #if 0
    printf("POPPED (%f, %f)\n",
           A->pairs[A->T].value,
           A->pairs[A->T].aggregate);
    #endif
    A->T = next(A, A->T); // discard last element
    if(A->T == A->H){ // empty
        return 0;
    }

    // Start from head and walk to tail, while updating the aggregate
    //printf("REBUILD INPUT\n");
    //aggregate_print(A);
    if(rebuild){
        u64 idx = previous(A, A->H);
        A->U = idx;
        A->pairs[idx].aggregate = A->pairs[idx].value;
        //printf("Rebuilding, starting at %lu until %lu\n", idx, A->T);
        while(idx != A->T){
            u64 idx2 = previous(A, idx);
            //printf("idx %lu vs %lu\n", idx, idx2);
            A->pairs[idx2].aggregate = min_double(A->pairs[idx].aggregate,
                                                 A->pairs[idx2].value);
            idx = idx2;
        }
        //printf("REBUILD OUTPUT\n");
        //aggregate_print(A);
    }

    return 0;
}

int aggregate_eval(const aggregate * A, double  *value){
    if(A->H == A->T){
        return -1; // empty
    }
    // finalize(combine(head, tail))
    *value = min_double(
                        A->pairs[previous(A, A->H)].aggregate,
                        A->pairs[A->T].aggregate);
    return 0;
}

void aggregate_reset(aggregate * A){
    A->H = 0;
    A->T = 0;
    A->U = 0;
    #ifndef NDEBUG
    memset(A->pairs, 0, A->capacity*sizeof(aggregate_pair));
    #endif
    return;
}

// Print by insertion order,
// i.e. tail element first and head element last
void aggregate_print(const aggregate * A){
    u64 pos = A->T;
    double minvalue = A->pairs[A->T].value;
    while(pos != A->H){
        printf("%lu (%.1f, %.1f)", pos,
               A->pairs[pos].value,
               A->pairs[pos].aggregate);
        if(A->pairs[pos].value < minvalue){
            minvalue = A->pairs[pos].value;
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
    }
    printf("---\n");
    double a_minvalue;
    aggregate_eval(A, &a_minvalue);
    assert(a_minvalue  == minvalue);
}
