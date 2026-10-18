#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <time.h>
#include <sched.h>
#include <unistd.h>
#include <x86intrin.h>

#include "gagg.h"
typedef uint64_t u64;
typedef uint32_t u32;

// pin the program to a specific cpu. 0-based id
static void pin_to_cpu(size_t cpu_id){
    cpu_set_t * set = CPU_ALLOC(cpu_id+1);
    assert(set != NULL);
    CPU_ZERO_S(CPU_ALLOC_SIZE(cpu_id+1), set);
    CPU_SET(cpu_id, set);
    if (sched_setaffinity(getpid(), sizeof(&set), set) == -1){
        fprintf(stderr, "sched_setaffinity failed\n");
        exit(EXIT_FAILURE);
    }
    CPU_FREE(set);
    if( sched_yield() ){ // switch if not on that cpu
        fprintf(stderr, "sched_yield failed\n");
        exit(EXIT_FAILURE);
    }
}


static double
timespec_diff(struct timespec* end, struct timespec * start)
{
    double elapsed = (end->tv_sec - start->tv_sec);
    elapsed += (end->tv_nsec - start->tv_nsec) / 1000000000.0;
    return elapsed;
}


void lift_min(double * dest, double * value)
{
    *dest = *value;
}

void lower_min(double * res, double * mon)
{
    *res = *mon;
}

void combine_min(double * target, double * left, double * right)
{
    if(*left < *right){
        *target = *left;
    } else {
        *target = *right;
    }
}

void print_min(double * v){
    printf("%f", *v);
}

void min_example(void){
    u64 w = 3;
    double unit = INFINITY;
    gagg * G = gagg_new(w,
                        sizeof(double),
                        &unit,
                        (gagg_lift_fcn) lift_min,
                        (gagg_lower_fcn) lower_min,
                        (gagg_combine_fcn) combine_min,
                        (gagg_print_fcn) print_min);
    double  res;
    gagg_eval(G, &res);
    printf("res = %f\n", res);
    for(u64 kk = 0; kk < w; kk++){
        double x = (double) rand() / (double) RAND_MAX;
        printf("PUSH(%f), ", x);
        gagg_push(G, &x);
        gagg_eval(G, &res);
        printf("EVAL=>%f,\n", res);
    }
    for(u64 kk = 0; kk < 2*w; kk++){
        printf("POP(), ");
        gagg_pop(G);
        double x = (double) rand() / (double) RAND_MAX;
        printf("PUSH(%f), ", x);
        gagg_push(G, &x);
        gagg_eval(G, &res);
        printf("EVAL=>%f,\n", res);
    }
    gagg_free(G);
    return;
}

void min_bench(int verbose, int w, double timeout){
    struct timespec tstart, tnow;
    u64 t0, t1;
    u32 cpuid;
    u64 n_ppe = 0;
    u64 t_ppe = 0;
    double unit = INFINITY;
    gagg * A = gagg_new(w,
                        sizeof(double),
                        &unit,
                        (gagg_lift_fcn) lift_min,
                        (gagg_lower_fcn) lower_min,
                        (gagg_combine_fcn) combine_min,
                        (gagg_print_fcn) print_min);

    if(verbose > 1){
        printf("Fill the buffer\n");
    }
    for(int kk = 0; kk < w; kk++){
        double v = 10-kk;
        gagg_push(A, &v);
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
        gagg_pop(A);
        gagg_push(A, &v);
        gagg_eval(A, &r);
        t1 = __rdtscp(&cpuid);
        res += r;
        t_ppe += t1 - t0;
        clock_gettime(CLOCK_REALTIME, &tnow);
        dt = timespec_diff(&tnow, &tstart);
        n_ppe++;
    }
    printf("\r %.1e\r                ", res);
    printf("Avg. PPE time: %.1f ticks\n", (double) t_ppe / (double) n_ppe);
    gagg_free(A);
}


typedef struct {
    u64 n;
    double S;
    double S2;
} std_mono;

void lift_std(std_mono * dest, double * value)
{
    dest->n = 1;
    dest->S = *value;
    dest->S2 = *value * *value;
}

void lower_std(double * res, std_mono * mon)
{
    if(mon->n == 0){
        *res = 0;
        return;
    }

    *res = sqrt( 1.0 / (double) mon->n * (mon->S2 - mon->S*mon->S/(double) mon->n));
    return;
}

void combine_std(std_mono * target, std_mono * left, std_mono * right)
{
    target->n = left->n + right->n;
    target->S = left->S + right->S;
    target->S2 = left->S2 + right->S2;

}
void print_std(std_mono * m){
    printf(".n=%lu, .S=%f, .S2=%f", m->n, m->S, m->S2);
}


void std_example(void){
    u64 w = 3;
    std_mono unit = {.n = 0, .S = 0, .S2 = 0};
    gagg * G = gagg_new(w,
                        sizeof(std_mono),
                        &unit,
                        (gagg_lift_fcn) lift_std,
                        (gagg_lower_fcn) lower_std,
                        (gagg_combine_fcn) combine_std,
                        (gagg_print_fcn) print_std);
    double res;
    gagg_eval(G, &res);
    printf("res = %f\n", res);
    for(u64 kk = 0; kk < w; kk++){
        double x = (double) rand() / (double) RAND_MAX;
        printf("PUSH(%f), ", x);
        gagg_push(G, &x);
        gagg_eval(G, &res);
        printf("EVAL=>%f,\n", res);
    }
    for(u64 kk = 0; kk < 2*w; kk++){
        printf("POP(), ");
        gagg_pop(G);
        double x = (double) rand() / (double) RAND_MAX;
        printf("PUSH(%f), ", x);
        gagg_push(G, &x);
        gagg_eval(G, &res);
        printf("EVAL=>%f,\n", res);
    }
    gagg_free(G);
    return;
}


int main(int argc, char ** argv)
{
    if(argc > 1){
        printf("%s takes no arguments\n", argv[0]);
    }

    min_example();
    std_example();
    pin_to_cpu(0);
    min_bench(1, 2<<14, 10);
    return EXIT_SUCCESS;
}
