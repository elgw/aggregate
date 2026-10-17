#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

    // The interface looks like a FIFO-queue with these methods:
    //
    // - **push** -- add to the accumulator. Use one combine operation.
    // - **eval** -- get the current estimate. Requires one combine operation.
    // - **pop** -- remove the first inserted element. Uses in average O(1) operations.
    //
    // What is different here is that the eval operation gives the statistics
    // the code was compiled for.
    //
    // Can be compile for one statistics at a time, please define one of
    // the following options:
    //
    // AGGREGATE_MIN    -- For min value
    // AGGREGATE_MEAN   -- For median value
    // AGGREGATE_STDDEV -- For standard deviation

    struct agg_min_struct;
    struct agg_mean_struct;
    struct agg_std_struct;
    typedef struct agg_min_struct agg_min_;
    typedef struct agg_mean_struct agg_mean_;
    typedef struct agg_std_struct agg_std_;

    //  What you would expect. It would of course be possible to
    // make the structure dynamic, i.e. let it grow whenever needed.
    agg_min_ * agg_min_new(uint64_t max_window_size);
    agg_mean_ * agg_mean_new(uint64_t max_window_size);
    agg_std_ * agg_std_new(uint64_t max_window_size);

    void agg_min_free(agg_min_ *);
    void agg_mean_free(agg_mean_ *);
    void agg_std_free(agg_std_ *);

    int agg_min_push(agg_min_ *, double value);
    int agg_mean_push(agg_mean_ *, double value);
    int agg_std_push(agg_std_ *, double value);

    int agg_min_pop(agg_min_ *);
    int agg_mean_pop(agg_mean_ *);
    int agg_std_pop(agg_std_ *);

    int agg_min_eval(const agg_min_ *, double * value);
    int agg_mean_eval(const agg_mean_ *, double * value);
    int agg_std_eval(const agg_std_ *, double * value);

    // Utility methods

    // Clear the contents of the AGGT(agg), i.e. reset as if brand new
    void agg_min_reset(agg_min_ *);
    void agg_mean_reset(agg_mean_ *);
    void agg_std_reset(agg_std_ *);

    // Print some diagnostics to the screen
    void agg_min_print(const agg_min_ *);
    void agg_mean_print(const agg_mean_ *);
    void agg_std_print(const agg_std_ *);

#ifdef __cplusplus
}
#endif
