#include "experiment.h"

#include <assert.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>

#define FEATURE_COUNT 4U
#define HASH_LENGTH 3U

static BioHashConfig test_config(void)
{
    BioHashConfig config = {
        .p = 499U,
        .q = 547U,
        .seed = 12345U,
        .threshold = 0.0,
        .orthogonality_tolerance = 1e-12
    };

    return config;
}

static void test_verification_experiment_retains_scores(void)
{
    const double features[4U][FEATURE_COUNT] = {
        {1.0, 0.0, 0.0, 0.0},
        {1.1, 0.0, 0.0, 0.0},
        {0.0, 1.0, 0.0, 0.0},
        {0.0, 1.1, 0.0, 0.0}
    };

    const size_t subject_ids[4U] = {
        1U,
        1U,
        2U,
        2U
    };

    const BioHashConfig config = test_config();

    VerificationExperimentResult result = {0};

    assert(
        run_verification_experiment(
            &features[0][0],
            subject_ids,
            4U,
            FEATURE_COUNT,
            HASH_LENGTH,
            &config,
            &result
        ) == EXPERIMENT_OK
    );

    assert(result.sample_count == 4U);
    assert(result.genuine_comparisons == 2U);
    assert(result.impostor_comparisons == 4U);

    assert(result.genuine_scores != NULL);
    assert(result.impostor_scores != NULL);

    double genuine_sum = 0.0;

    for (
        size_t index = 0U;
        index < result.genuine_comparisons;
        ++index
    ) {
        assert(result.genuine_scores[index] >= 0.0);
        assert(result.genuine_scores[index] <= 1.0);

        genuine_sum += result.genuine_scores[index];
    }

    double impostor_sum = 0.0;

    for (
        size_t index = 0U;
        index < result.impostor_comparisons;
        ++index
    ) {
        assert(result.impostor_scores[index] >= 0.0);
        assert(result.impostor_scores[index] <= 1.0);

        impostor_sum += result.impostor_scores[index];
    }

    const double expected_genuine_mean =
        genuine_sum /
        (double)result.genuine_comparisons;

    const double expected_impostor_mean =
        impostor_sum /
        (double)result.impostor_comparisons;

    assert(
        fabs(
            result.mean_genuine_distance -
            expected_genuine_mean
        ) < 1e-12
    );

    assert(
        fabs(
            result.mean_impostor_distance -
            expected_impostor_mean
        ) < 1e-12
    );

    assert(result.equal_error_rate.threshold >= 0.0);
    assert(result.equal_error_rate.threshold <= 1.0);

    assert(result.equal_error_rate.fmr >= 0.0);
    assert(result.equal_error_rate.fmr <= 1.0);

    assert(result.equal_error_rate.fnmr >= 0.0);
    assert(result.equal_error_rate.fnmr <= 1.0);

    assert(result.equal_error_rate.eer >= 0.0);
    assert(result.equal_error_rate.eer <= 1.0);

    verification_experiment_result_free(&result);

    assert(result.sample_count == 0U);
    assert(result.genuine_comparisons == 0U);
    assert(result.impostor_comparisons == 0U);

    assert(result.genuine_scores == NULL);
    assert(result.impostor_scores == NULL);
}

static void test_experiment_requires_both_score_classes(void)
{
    const double features[2U][FEATURE_COUNT] = {
        {1.0, 0.0, 0.0, 0.0},
        {0.0, 1.0, 0.0, 0.0}
    };

    const BioHashConfig config = test_config();

    VerificationExperimentResult result = {0};

    const size_t same_subject_ids[2U] = {
        1U,
        1U
    };

    assert(
        run_verification_experiment(
            &features[0][0],
            same_subject_ids,
            2U,
            FEATURE_COUNT,
            HASH_LENGTH,
            &config,
            &result
        ) == EXPERIMENT_INSUFFICIENT_COMPARISONS
    );

    const size_t different_subject_ids[2U] = {
        1U,
        2U
    };

    assert(
        run_verification_experiment(
            &features[0][0],
            different_subject_ids,
            2U,
            FEATURE_COUNT,
            HASH_LENGTH,
            &config,
            &result
        ) == EXPERIMENT_INSUFFICIENT_COMPARISONS
    );

    verification_experiment_result_free(&result);
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

    VerificationExperimentResult result = {0};

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
            NULL,
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

    assert(
        run_verification_experiment(
            &features[0][0],
            subject_ids,
            2U,
            FEATURE_COUNT,
            HASH_LENGTH,
            NULL,
            &result
        ) == EXPERIMENT_INVALID_ARGUMENT
    );

    assert(
        run_verification_experiment(
            &features[0][0],
            subject_ids,
            2U,
            FEATURE_COUNT,
            HASH_LENGTH,
            &config,
            NULL
        ) == EXPERIMENT_INVALID_ARGUMENT
    );
}

static void test_result_free_is_safe(void)
{
    verification_experiment_result_free(NULL);

    VerificationExperimentResult result = {0};

    verification_experiment_result_free(&result);

    assert(result.genuine_scores == NULL);
    assert(result.impostor_scores == NULL);
    assert(result.sample_count == 0U);
}

int main(void)
{
    test_verification_experiment_retains_scores();
    test_experiment_requires_both_score_classes();
    test_invalid_arguments_are_rejected();
    test_result_free_is_safe();

    printf(
        "All verification experiment tests passed.\n"
    );

    return 0;
}
