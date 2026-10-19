#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdint.h>

    struct gagg_struct;
    typedef struct gagg_struct gagg;

    typedef void (*gagg_lift_fcn)(void * dest, const void * source);
    typedef void (*gagg_lower_fcn)(void * dest, const void * source);
    typedef void (*gagg_combine_fcn)(void * dest, void * L, void * R);
    typedef void (*gagg_print_fcn)(const void *);
    gagg * gagg_new(uint64_t w,
                    uint64_t mon_size, // size of lifted element, in bytes
                    void * mon_unit, // the unit element in the monoid
                    gagg_lift_fcn lift,
                    gagg_lower_fcn lower,
                    gagg_combine_fcn combine,
                    gagg_print_fcn print);
    void gagg_free(gagg *);
    int gagg_push(gagg * A, void * val);
    int gagg_eval(const gagg * A, void  *val);
    int gagg_pop(gagg * A);

#ifdef __cplusplus
}
#endif
