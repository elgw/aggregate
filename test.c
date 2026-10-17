#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <x86intrin.h>
#include <math.h>

#include "aggregate.h"

typedef uint64_t u64;
typedef uint32_t u32;

void test_mean(int w){
    const int verbose = 1;
    if(verbose > 0){
        printf("AGG_MEAN_MEAN: Testing with window size = %d\n", w);
    }
    agg_mean_ * AGG = agg_mean_new(w);

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
        agg_mean_push(AGG, v);
        if(verbose > 2){
            agg_mean_print(AGG);
        }
        double r;
        agg_mean_eval(AGG, &r);
        assert(v == r);
        if(verbose > 1){
            printf("Popping\n");
        }
        if(agg_mean_pop(AGG)){
            assert(0);
        }
        if(verbose > 2){
            agg_mean_print(AGG);
        }
    }

    agg_mean_reset(AGG);
    if(verbose > 1){
        printf("Fill the buffer\n");
    }
    for(int kk = 0; kk < w; kk++){
        agg_mean_push(AGG, 10.0 - kk);
        if(verbose > 2){
            agg_mean_print(AGG);
        }
    }

    u64 n_operations = 0;
    u64 t_ppe = 0;

    if(verbose > 1){
        printf("EVAL_POP_PUSH cycle\n");
    }
    int n_test = 10000000 / w;
    double r;

    for(int kk = 0; kk < n_test; kk++){
        double sum = 0;
        double rsum = 0;
        for(int vv = 0; vv < w; vv++){
            n_operations++;
            double v = 10.0 * (double) rand() / (double) RAND_MAX;
            sum += v;
            t0 = __rdtscp(&cpuid);
            agg_mean_pop(AGG);
            agg_mean_push(AGG, v);
            agg_mean_eval(AGG, &r);
            t1 = __rdtscp(&cpuid);
            t_ppe += t1 - t0;
            rsum += r;
        }
        printf("\r%f", rsum);


        if( fabs(r - sum / (double) w) > 1e-5 ){
            printf("ERROR:\n");
            printf("Expected %f\n", sum / (double) w);
            printf("Got:     %f\n", r);
            agg_mean_print(AGG);
            assert(0);
            exit(EXIT_FAILURE);
        }
    }
    printf("   Results after %lu operations\n", n_operations);
    printf("   Avg. PPE Time: %.1f\n", (double) t_ppe / (double) n_operations);

    agg_mean_free(AGG);
}

void show_steps(int w){
    printf("agg_mean_new(%d)\n", w);
    agg_mean_ * A = agg_mean_new(w);
    agg_mean_print(A);

    for(int kk = 0; kk < 5; kk++){
        double v = (double) rand() / (double) RAND_MAX;
        printf("agg_mean_push(A, %f)\n", v);
        agg_mean_push(A, v);
        agg_mean_print(A);
    }

    for(int kk = 0; kk < 5; kk++){
        double v;
        if(agg_mean_eval(A, &v)){
            assert(0);
        }
        printf("agg_mean_eval(A) -> %f\n", v);
        printf("agg_mean_pop(A)\n");
        agg_mean_pop(A);
        agg_mean_print(A);
        v = (double) rand() / (double) RAND_MAX;
        printf("agg_mean_push(A, %f)\n", v);
        agg_mean_push(A, v);
        agg_mean_print(A);
    }
    printf("Emptying the aggrate\n");
    for(int kk = 0; kk < 5; kk ++){
        double v;
        if(agg_mean_eval(A, &v)){
            assert(0);
        }
        printf("agg_mean_eval(A) -> %f\n", v);
        printf("agg_mean_pop(A)\n");
        agg_mean_pop(A);
        agg_mean_print(A);
    }
    agg_mean_free(A);
    A = NULL;
    return;
}


int main(int argc, char ** argv){
    if(argc > 1){
        printf("%s takes no arguments\n", argv[0]);
    }
    show_steps(5);
    test_mean(16384); // 2^14
    return EXIT_SUCCESS;

    for(int w = 3; w < 121; w++){
        test_mean(w);
    }
    return EXIT_SUCCESS;
}
