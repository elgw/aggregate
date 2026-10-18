#include "bench.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>
#include <time.h>

#include "aggregate.h"

typedef uint64_t u64;
typedef uint32_t u32;

#ifdef AGGREGATE_MIN
#define AGGT(x) agg_min_##x
#endif

#ifdef AGGREGATE_MEAN
#define AGGT(x) agg_mean_##x
#endif

#ifdef AGGREGATE_STD
#define AGGT(x) agg_std_##x
#endif

static double
timespec_diff(struct timespec* end, struct timespec * start)
{
    double elapsed = (end->tv_sec - start->tv_sec);
    elapsed += (end->tv_nsec - start->tv_nsec) / 1000000000.0;
    return elapsed;
}


void AGGT(ut)(int verbose, int w)
{
    if(verbose > 1){
        printf("%d\n", w);
    }
    u64 n = 3*w + (u64) rand() % w;
    double * V = malloc(n*sizeof(double));
    // TODO !
    free(V);
}

void AGGT(bench)(int verbose, int w, double timeout){
    struct timespec tstart, tnow;
    u64 t0, t1;
    u32 cpuid;
    u64 n_ppe = 0;
    u64 t_ppe = 0;
    AGGT() * A = AGGT(new)(w);
    if(verbose > 1){
        printf("Fill the buffer\n");
    }
    for(int kk = 0; kk < w; kk++){
        AGGT(push)(A, 10.0 - kk);
    }

    if(verbose > 1){
        printf("POP-PUSH-EVAL cycle\n");
    }
    clock_gettime(CLOCK_REALTIME, &tstart);
    volatile double res = 0;
    double dt = 0;
    while(dt < timeout){
        double v = 10.0 * (double) rand() / (double) RAND_MAX;
        double r;
        t0 = __rdtscp(&cpuid);
        AGGT(pop)(A);
        AGGT(push)(A, v);
        AGGT(eval)(A, &r);
        t1 = __rdtscp(&cpuid);
        res += r;
        t_ppe += t1 - t0;
        clock_gettime(CLOCK_REALTIME, &tnow);
        dt = timespec_diff(&tnow, &tstart);
        n_ppe++;
    }
    printf("\r %.1e\r                ", res);
    printf("Avg. PPE time: %.1f ticks\n", (double) t_ppe / (double) n_ppe);
    AGGT(free)(A);
}

void AGGT(steps)(int w){
    printf("new, w=%d\n", w);
    AGGT() * A = AGGT(new)(w);
    AGGT(print)(A);

    printf("Filling ...\n");
    for(int kk = 0; kk < 5; kk++){
        double v = (double) rand() / (double) RAND_MAX;
        printf("push(A, %f)\n", v);
        AGGT(push)(A, v);
        AGGT(print)(A);
    }

    printf("POP-PUSH-EVAL ...\n");
    for(int kk = 0; kk < 5; kk++){
        printf("pop(A)\n");
        AGGT(pop)(A);
        AGGT(print)(A);
        double v = (double) rand() / (double) RAND_MAX;
        printf("push(A, %f)\n", v);
        AGGT(push)(A, v);
        AGGT(print)(A);
        if(AGGT(eval)(A, &v)){
            assert(0);
        }
        printf("eval(A) -> %f\n", v);
    }
    printf("Emptying ...\n");
    for(int kk = 0; kk < 5; kk ++){
        double v;
        if(AGGT(eval)(A, &v)){
            assert(0);
        }
        printf("eval(A) -> %f\n", v);
        printf("pop(A)\n");
        AGGT(pop)(A);
        AGGT(print)(A);
    }
    AGGT(free)(A);
    A = NULL;
    return;
}
