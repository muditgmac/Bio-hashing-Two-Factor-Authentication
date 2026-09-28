#include <assert.h>
#include <stdio.h>

#include "experiment.h"

#define SAMPLE_COUNT 4U
#define FEATURE_COUNT 4U
#define HASH_LENGTH 1U

static BioHashConfig test_config(void)
{
    const BioHashConfig config = {
        .p = 7U,
        .q = 11U,
        .seed = 5U,
        .threshold = 0.0,
        .orthogonality_tolerance = 1e-12
    };

    return config;
}

static void test_verification_experiment_runs(void)
{
    /*
     * Two samples belong to subject 1.
     * Two samples belong to subject 2.
     *
     * With four samples:
     *
     * total pairs    = 6
     * genuine pairs  = 2
     * impostor pairs = 4
     */
    const double features[SAMPLE_COUNT][FEATURE_COUNT] = {
        { 1.00,  0.20,  0.10,  0.00},
        { 0.95,  0.18,  0.12,  0.02},
        {-1.00, -0.20, -0.10,  0.00},
        {-0.95, -0.18, -0.12, -0.02}
    };

    const size_t subject_ids[SAMPLE_COUNT] = {
        1U,
        1U,
        2U,
        2U
    };

    const BioHashConfig config = test_config();

    VerificationExperimentResult result;

    const ExperimentStatus status =
        run_verification_experiment(
            &features[0][0],
            subject_ids,
            SAMPLE_COUNT,
            FEATURE_COUNT,
            HASH_LENGTH,
            &config,
            &result
        );

    assert(status == EXPERIMENT_OK);

    assert(result.sample_count == SAMPLE_COUNT);
    assert(result.genuine_comparisons == 2U);
    assert(result.impostor_comparisons == 4U);

    assert(result.mean_genuine_distance >= 0.0);
    assert(result.mean_genuine_distance <= 1.0);

    assert(result.mean_impostor_distance >= 0.0);
    assert(result.mean_impostor_distance <= 1.0);

    assert(result.equal_error_rate.threshold >= 0.0);
    assert(result.equal_error_rate.threshold <= 1.0);

    assert(result.equal_error_rate.eer >= 0.0);
    assert(result.equal_error_rate.eer <= 1.0);

    assert(result.equal_error_rate.fmr >= 0.0);
    assert(result.equal_error_rate.fmr <= 1.0);

    assert(result.equal_error_rate.fnmr >= 0.0);
    assert(result.equal_error_rate.fnmr <= 1.0);
}

static void test_experiment_requires_genuine_and_impostor_pairs(void)
{
    const double features[SAMPLE_COUNT][FEATURE_COUNT] = {
        {1.0, 0.0, 0.0, 0.0},
        {0.9, 0.1, 0.0, 0.0},
        {0.8, 0.2, 0.0, 0.0},
        {0.7, 0.3, 0.0, 0.0}
    };

    const size_t same_subject[SAMPLE_COUNT] = {
        1U,
        1U,
        1U,
        1U
    };

    const BioHashConfig config = test_config();

    VerificationExperimentResult result;

    assert(
        run_verification_experiment(
            &features[0][0],
            same_subject,
            SAMPLE_COUNT,
            FEATURE_COUNT,
            HASH_LENGTH,
            &config,
            &result
        ) == EXPERIMENT_INSUFFICIENT_COMPARISONS
    );
}

static void test_invalid_arguments_are_rejected(void)
{
    const double features[2U][FEATURE_COUNT] = {
        {1.0, 0.0, 0.0, 0.0},
        {0.0, 1.0, 0.0, 0.0}
    };

    const size_t subject_ids[2U] = {
        1U,
        2U
    };

    const BioHashConfig config = test_config();

    VerificationExperimentResult result;

    assert(
        run_verification_experiment(
            NULL,
            subject_ids,
            2U,
            FEATURE_COUNT,
            HASH_LENGTH,
            &config,
            &result
        ) == EXPERIMENT_INVALID_ARGUMENT
    );

    assert(
        run_verification_experiment(
            &features[0][0],
            subject_ids,
            1U,
            FEATURE_COUNT,
            HASH_LENGTH,
            &config,
            &result
        ) == EXPERIMENT_INVALID_ARGUMENT
    );

    assert(
        run_verification_experiment(
            &features[0][0],
            subject_ids,
            2U,
            FEATURE_COUNT,
            FEATURE_COUNT + 1U,
            &config,
            &result
        ) == EXPERIMENT_INVALID_ARGUMENT
    );
}

int main(void)
{
    test_verification_experiment_runs();
    test_experiment_requires_genuine_and_impostor_pairs();
    test_invalid_arguments_are_rejected();

    printf("All verification experiment tests passed.\n");

    return 0;
}
