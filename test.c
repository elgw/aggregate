#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <x86intrin.h>

#include "aggregate.h"

typedef uint64_t u64;
typedef uint32_t u32;

static double min_double(double a, double b){
    if(a < b){
        return a;
    }
    return b;
}

void test(int w){
    int verbose =  1;
    if(verbose > 0){
        printf("Testing with window size = %d\n", w);
    }
    aggregate * AGG = aggregate_new(w);

    u64 t0, t1;
    u32 cpuid;

    if(verbose > 1){
        printf("Add and remove one element\n");
    }
    for(int kk = 0; kk < 3*w; kk++){
        double v = kk+1.1;
        if(verbose > 1){
            printf("Adding %f\n", v);
        }
        aggregate_push(AGG, v);
        if(verbose > 2){
            aggregate_print(AGG);
        }
        double r;
        aggregate_eval(AGG, &r);
        assert(v == r);
        if(verbose > 1){
            printf("Popping\n");
        }
        if(aggregate_pop(AGG)){
            assert(0);
        }
        if(verbose > 2){
            aggregate_print(AGG);
        }
    }

    aggregate_reset(AGG);
    if(verbose > 1){
        printf("Fill the buffer\n");
    }
    for(int kk = 0; kk < w; kk++){
        aggregate_push(AGG, 10.0 - kk);
        if(verbose > 2){
            aggregate_print(AGG);
        }
    }

    u64 n_operations = 0;
    u64 t_pop = 0;
    u64 t_push = 0;

    if(verbose > 1){
        printf("EVAL_POP_PUSH cycle\n");
    }
    int n_test = 100000;
    for(int kk = 0; kk < n_test; kk++){
        double minvalue = 1e99;
        for(int vv = 0; vv < w; vv++){
            n_operations++;
            t0 = __rdtscp(&cpuid);
            aggregate_pop(AGG);
            t1 = __rdtscp(&cpuid);
            //printf("After pop\n");
            //aggregate_print(AGG);
            t_pop += t1 - t0;

            double v = 10.0 * (double) rand() / (double) RAND_MAX;
            minvalue = min_double(minvalue, v);
            //double v = (double) kk;
            t0 = __rdtscp(&cpuid);
            aggregate_push(AGG, v);
            t1 = __rdtscp(&cpuid);
            t_push += t1 - t0;
            //printf("After push\n");
            //aggregate_print(AGG);
            //aggregate_print(AGG);
        }
        double r;
        aggregate_eval(AGG, &r);
        assert( r == minvalue );
    }
    printf("   Results after %lu operations\n", n_operations);
    printf("   Avg. Push Time: %.1f\n", (double) t_push / (double) n_operations);
    printf("   Avg. Pop Time:  %.1f\n", (double) t_pop  / (double) n_operations);

    aggregate_free(AGG);
}


int main(int argc, char ** argv){
    if(argc > 1){
        printf("%s takes no arguments\n", argv[0]);
    }

    for(int w = 3; w < 121; w++){
        test(w);
    }
    return EXIT_SUCCESS;
}
