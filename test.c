#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <x86intrin.h>
#include <math.h>
#include <getopt.h>

#include "aggregate.h"

typedef uint64_t u64;
typedef uint32_t u32;

static double min_double(double a, double b){
    if(a < b){
        return a;
    }
    return b;
}

void test_min(int verbose, int w){
    if(verbose > 0){
        printf("AGG_MIN_MIN: Testing with window size = %d\n", w);
    }
    agg_min_ * AGG = agg_min_new(w);

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
        agg_min_push(AGG, v);
        if(verbose > 2){
            agg_min_print(AGG);
        }
        double r;
        agg_min_eval(AGG, &r);
        assert(v == r);
        if(verbose > 1){
            printf("Popping\n");
        }
        if(agg_min_pop(AGG)){
            assert(0);
        }
        if(verbose > 2){
            agg_min_print(AGG);
        }
    }

    agg_min_reset(AGG);
    if(verbose > 1){
        printf("Fill the buffer\n");
    }
    for(int kk = 0; kk < w; kk++){
        agg_min_push(AGG, 10.0 - kk);
        if(verbose > 2){
            agg_min_print(AGG);
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
            agg_min_pop(AGG);
            agg_min_push(AGG, v);
            agg_min_eval(AGG, &r);
            t1 = __rdtscp(&cpuid);
            t_ppe += t1 - t0;
            printf("\r%.1f", r);
        }
        agg_min_eval(AGG, &r);
        assert( r == minvalue );
    }
    printf("\r   Results after %lu operations\n", n_operations);
    printf("   Avg. PPE Time: %.1f\n", (double) t_ppe / (double) n_operations);

    agg_min_free(AGG);
}

void test_mean(int verbose, int w){
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

void test_std(int verbose, int w){
    if(verbose > 0){
        printf("AGG_STD_STDDEV: Testing with window size = %d\n", w);
    }
    agg_std_ * AGG = agg_std_new(w);

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
        agg_std_push(AGG, v);
        if(verbose > 2){
            agg_std_print(AGG);
        }
        double r;
        agg_std_eval(AGG, &r);
        assert(v == r);
        if(verbose > 1){
            printf("Popping\n");
        }
        if(agg_std_pop(AGG)){
            assert(0);
        }
        if(verbose > 2){
            agg_std_print(AGG);
        }
    }

    agg_std_reset(AGG);
    if(verbose > 1){
        printf("Fill the buffer\n");
    }
    for(int kk = 0; kk < w; kk++){
        agg_std_push(AGG, 10.0 - kk);
        if(verbose > 2){
            agg_std_print(AGG);
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
            agg_std_pop(AGG);
            agg_std_push(AGG, v);
            agg_std_eval(AGG, &r);
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
            //agg_std_print(AGG);
            //assert(0);
            //exit(EXIT_FAILURE);
        }
    }
    printf("\r   Results after %lu operations\n", n_operations);
    printf("   Avg. PPE Time: %.1f\n", (double) t_ppe / (double) n_operations);

    agg_std_free(AGG);
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

typedef struct {
    int verbose;
    int method; // 0=all, 1 = min, 2 = mean, 3 = std
    int benchmark; // yes/no
    int validate; // yes/no
    int steps; // yes/no
} config;

static void usage(void){
    printf("--verbose v\n");
    printf("--method n\n");
    printf("--benchmark\n");
    printf("--validate\n");
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
        { NULL,           0,               NULL,  0  }
    };
    config * conf = calloc(1, sizeof(config));
    conf->verbose = 1;
    int ch;
    while((ch = getopt_long(argc, argv,
                            "vm:bVs",
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
        if( (conf->method == 0) | (conf->method == 1) ){
            test_min(conf->verbose, 16384); // 2^14
        }
        if( (conf->method == 0) | (conf->method == 2) ){
            test_mean(conf->verbose, 16384);
        }
        if( (conf->method == 0) | (conf->method == 3) ){
            test_std(conf->verbose, 16384);
        }
        free(conf);
        return EXIT_SUCCESS;
    }

    if(conf->steps){
        show_steps(5);
        free(conf);
        return EXIT_SUCCESS;
    }

    if(conf->validate){
        for(int w = 3; w < 121; w++){
            test_mean(conf->verbose, w);
        }
        free(conf);
        return EXIT_SUCCESS;
    }

    return EXIT_SUCCESS;
}
