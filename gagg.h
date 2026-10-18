#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdint.h>

    struct gagg_struct;
    typedef struct gagg_struct gagg;

    typedef void (*gagg_lift_fcn)(void *, void *);
    typedef void (*gagg_lower_fcn)(void *, void *);
    typedef void (*gagg_combine_fcn)(void *, void *, void *);
    typedef void (*gagg_print_fcn)(void *);

    gagg * gagg_new(uint64_t w,
                   uint64_t mon_size,
                   void * gagg_unit,
                   gagg_lift_fcn lift,
                   gagg_lower_fcn lower,
                   gagg_combine_fcn combine,
                   gagg_print_fcn print);
    void gagg_free(gagg *);
    int gagg_push(gagg * A, void * val);
    int gagg_eval(const gagg * A, void  *val);
    int gagg_pop(gagg * A);
