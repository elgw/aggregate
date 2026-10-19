#define _GNU_SOURCE
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <x86intrin.h>
#include <math.h>
#include <getopt.h>
#ifdef __linux__
#include <sched.h>
#endif
#include <unistd.h>

#include "aggregate.h"
#include "bench.h"

typedef uint64_t u64;
typedef uint32_t u32;

#ifdef __linux__
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
#else
static void pin_to_cpu(__attribute__((unused)) size_t cpu_id){
}
#endif

typedef struct {
    int verbose;
    int method; // 0=all, 1 = min, 2 = mean, 3 = std
    int benchmark; // yes/no
    int validate; // yes/no
    int steps; // yes/no
    double timeout;
} config;

static void usage(void){
    printf("--verbose v\n");
    printf("--method n\n");
    printf("--benchmark\n\t"
           "Benchmark all methods\n");
    printf("--timeout s\n\t"
           "Timeout value in seconds for the benchmarks\n");
    printf("--steps\n\t"
           "Show step by step internals\n");
    printf("--validate\n");
    printf("--help\n\tShow this message\n");
    return;
}

static config *
config_new(int argc, char ** argv)
{
    struct option longopts[] = {
        { "verbose",    required_argument, NULL, 'v' },
        { "method",     required_argument, NULL, 'm' },
        { "benchmark",  no_argument, NULL, 'b' },
        { "steps",      no_argument, NULL, 's' },
        { "validate",   no_argument, NULL, 'V'},
        { "help",       no_argument, NULL, 'h'},
        { "timeout",    required_argument, NULL, 't'},
        { NULL,           0,               NULL,  0  }
    };
    config * conf = calloc(1, sizeof(config));
    conf->verbose = 1;
    conf->timeout = 1.0; // s
    int ch;
    while((ch = getopt_long(argc, argv,
                            "vm:bVst:",
                            longopts, NULL)) != -1) {
        switch(ch) {
        case 'v':
            conf->verbose = atoi(optarg);
            break;
        case 'm':
            conf->method = atoi(optarg);
            break;
        case 'b':
            conf->benchmark = 1;
            break;
        case 'V':
            conf->validate = 1;
            break;
        case 'h':
            usage();
            exit(EXIT_SUCCESS);
        case 's':
            conf->steps = 1;
            break;
        case 't':
            conf->timeout = atof(optarg);
        default:
            break;
        }
    }
    return conf;
}


int main(int argc, char ** argv){
    config * conf = config_new(argc, argv);
    if(conf->verbose > 1){
        printf("conf->verbose = %d\n", conf->verbose);
        printf("conf->benchmark = %d\n", conf->benchmark);
        printf("conf->method = %d\n", conf->method);
        printf("conf->validate = %d\n", conf->validate);
    }

    if(conf->benchmark == 1){
        pin_to_cpu(0);
        u64 w = 16384; // 2^14
        if( (conf->method == 0) | (conf->method == 1) ){
            agg_min_bench(conf->verbose, w, conf->timeout);
        }
        if( (conf->method == 0) | (conf->method == 2) ){
            agg_mean_bench(conf->verbose, w, conf->timeout);
        }
        if( (conf->method == 0) | (conf->method == 3) ){
            agg_std_bench(conf->verbose, w, conf->timeout);
        }
        free(conf);
        return EXIT_SUCCESS;
    }

    if(conf->steps){
        u64 w = 5;
        if( (conf->method == 0) | (conf->method == 1) ){
            printf("Showing steps for min\n\n");
            agg_min_steps(w);
        }
        if( (conf->method == 0) | (conf->method == 2) ){
            printf("Showing steps for mean\n\n");
            agg_mean_steps(w);
        }
        if( (conf->method == 0) | (conf->method == 3) ){
            printf("Showing steps for std\n\n");
            agg_std_steps(w);
        }
        free(conf);
        return EXIT_SUCCESS;
    }

    // Linear scan comparison vs naiive method on an array of 3*w +
    // (u64) rand() % w elements

    if(conf->validate){
        for(u64 w = 1; w < 117; w++){
            if( (conf->method == 0) | (conf->method == 1) ){
                if(conf->verbose > 0){
                    printf("Validating min, w=%ld\n", w);
                }
                agg_min_ut(conf->verbose, w);
            }
            if( (conf->method == 0) | (conf->method == 2) ){
                if(conf->verbose > 0){
                    printf("Validating mean, w=%ld\n", w);
                }
                agg_mean_ut(conf->verbose, w);
            }
            if( (conf->method == 0) | (conf->method == 3) ){
                if(conf->verbose > 0){
                    printf("Validating std, w=%ld\n", w);
                }
                agg_std_ut(conf->verbose, w);
            }
        }
        free(conf);
        return EXIT_SUCCESS;
    }

    return EXIT_SUCCESS;
}
