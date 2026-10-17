#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <x86intrin.h>
#include <math.h>

#include "aggregate.h"

typedef uint64_t u64;
typedef uint32_t u32;

void test_stddev(int w){
    const int verbose = 1;
    if(verbose > 0){
        printf("AGGREGATE_STDDEV: Testing with window size = %d\n", w);
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

    int n_test = 10000000 / w;
    double r;

    for(int kk = 0; kk < n_test; kk++){
        double sum = 0;
        double sum2 = 0;

        double rsum = 0;
        for(int vv = 0; vv < w; vv++){
            n_operations++;
            double v = 10.0 * (double) rand() / (double) RAND_MAX;
            sum += v;
            sum2 += v*v;
            t0 = __rdtscp(&cpuid);
            aggregate_pop(AGG);
            aggregate_push(AGG, v);
            aggregate_eval(AGG, &r);
            t1 = __rdtscp(&cpuid);
            t_ppe += t1 - t0;
            rsum += r;
        }
        printf("\r%.1f            ", rsum);
        double ref = sqrt((1.0 / (double) w) *(sum2 - sum*sum));

        if( fabs(r - ref) > 1e-5 ){
            printf("\rERROR:               \n");
            printf("Expected %f\n", sum / (double) w);
            printf("Got:     %f\n", r);
            //aggregate_print(AGG);
            //assert(0);
            //exit(EXIT_FAILURE);
        }
    }
    printf("\r   Results after %lu operations\n", n_operations);
    printf("   Avg. PPE Time: %.1f\n", (double) t_ppe / (double) n_operations);

    aggregate_free(AGG);
}


int main(int argc, char ** argv){
    if(argc > 1){
        printf("%s takes no arguments\n", argv[0]);
    }

    test_stddev(16384); // 2^14
    return EXIT_SUCCESS;

    for(int w = 3; w < 121; w++){
        test_stddev(w);
    }
    return EXIT_SUCCESS;
}
