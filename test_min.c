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

void test_min(int w){
    int verbose =  1;
    if(verbose > 0){
        printf("AGGREGATE_MIN: Testing with window size = %d\n", w);
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
    u64 t_ppe = 0;

    if(verbose > 1){
        printf("EVAL_POP_PUSH cycle\n");
    }
    int n_test = 10000000/w;
    double r;
    for(int kk = 0; kk < n_test; kk++){
        double minvalue = 1e99;
        for(int vv = 0; vv < w; vv++){
            n_operations++;
            double v = 10.0 * (double) rand() / (double) RAND_MAX;
            minvalue = min_double(minvalue, v);

            t0 = __rdtscp(&cpuid);
            aggregate_pop(AGG);
            aggregate_push(AGG, v);
            aggregate_eval(AGG, &r);
            t1 = __rdtscp(&cpuid);
            t_ppe += t1 - t0;
            printf("\r%.1f", r);
        }
        aggregate_eval(AGG, &r);
        assert( r == minvalue );
    }
    printf("\r   Results after %lu operations\n", n_operations);
    printf("   Avg. PPE Time: %.1f\n", (double) t_ppe / (double) n_operations);

    aggregate_free(AGG);
}

void show_steps(int w){
    printf("aggregate_new(%d)\n", w);
    aggregate * A = aggregate_new(w);
    aggregate_print(A);

    for(int kk = 0; kk < 5; kk++){
        double v = (double) rand() / (double) RAND_MAX;
        printf("aggregate_push(A, %f)\n", v);
        aggregate_push(A, v);
        aggregate_print(A);
    }

    for(int kk = 0; kk < 5; kk++){
        double v;
        if(aggregate_eval(A, &v)){
            assert(0);
        }
        printf("aggregate_eval(A) -> %f\n", v);
        printf("aggregate_pop(A)\n");
        aggregate_pop(A);
        aggregate_print(A);
        v = (double) rand() / (double) RAND_MAX;
        printf("aggregate_push(A, %f)\n", v);
        aggregate_push(A, v);
        aggregate_print(A);
    }
    printf("Emptying the aggrate\n");
    for(int kk = 0; kk < 5; kk ++){
        double v;
        if(aggregate_eval(A, &v)){
            assert(0);
        }
        printf("aggregate_eval(A) -> %f\n", v);
        printf("aggregate_pop(A)\n");
        aggregate_pop(A);
        aggregate_print(A);
    }
    aggregate_free(A);
    A = NULL;
    return;
}

int main(int argc, char ** argv){
    if(argc > 1){
        printf("%s takes no arguments\n", argv[0]);
    }
    //show_steps(5);
    test_min(16384);
    return EXIT_SUCCESS;

    for(int w = 3; w < 121; w++){
        test_min(w);
    }
    return EXIT_SUCCESS;
}
