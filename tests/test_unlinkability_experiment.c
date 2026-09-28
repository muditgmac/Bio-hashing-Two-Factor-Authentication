#include "unlinkability_experiment.h"

#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

#define SAMPLE_COUNT 4U
#define FEATURE_COUNT 8U
#define HASH_LENGTH 4U
#define TOKEN_COUNT 2U
#define BIN_COUNT 8U

/*
 * Two samples are provided for each of two subjects.
 *
 * The values themselves are deterministic synthetic feature vectors.
 * The purpose of this test is not to claim biometric performance, but
 * to verify that the complete unlinkability experiment pipeline
 * correctly connects template generation, cross-token comparison,
 * score-distribution construction, metric estimation and cleanup.
 */
static const double FEATURES[SAMPLE_COUNT][FEATURE_COUNT] = {
    {
        1.00, 0.25, -0.50, 0.75,
        0.10, -0.20, 0.30, -0.40
    },
    {
        0.95, 0.30, -0.45, 0.70,
        0.15, -0.25, 0.35, -0.35
    },
    {
        -0.70, 0.80, 0.20, -0.10,
        0.60, 0.40, -0.30, 0.50
    },
    {
        -0.65, 0.75, 0.25, -0.15,
        0.55, 0.45, -0.25, 0.55
    }
};

static const size_t SUBJECT_IDS[SAMPLE_COUNT] = {
    1U,
    1U,
    2U,
    2U
};

/*
 * 383 and 503 are distinct Blum primes:
 *
 *     383 mod 4 = 3
 *     503 mod 4 = 3
 *
 * Changing the seed models a different token while preserving the
 * remaining BioHash configuration.
 */
static BioHashConfig token_config(uint64_t seed)
{
    const BioHashConfig config = {
        .p = 383U,
        .q = 503U,
        .seed = seed,
        .threshold = 0.0,
        .orthogonality_tolerance = 1e-12
    };

    return config;
}

static void test_end_to_end_unlinkability_experiment_runs(void)
{
    const BioHashConfig configs[TOKEN_COUNT] = {
        token_config(101U),
        token_config(103U)
    };

    UnlinkabilityExperimentResult result = {0};

    const UnlinkabilityExperimentStatus status =
        run_unlinkability_experiment(
            &FEATURES[0][0],
            SUBJECT_IDS,
            SAMPLE_COUNT,
            FEATURE_COUNT,
            HASH_LENGTH,
            configs,
            TOKEN_COUNT,
            BIN_COUNT,
            1.0,
            &result
        );

    assert(status == UNLINKABILITY_EXPERIMENT_OK);

    /*
     * Verify top-level experiment metadata.
     */
    assert(result.sample_count == SAMPLE_COUNT);
    assert(result.feature_count == FEATURE_COUNT);
    assert(result.hash_length == HASH_LENGTH);
    assert(result.token_count == TOKEN_COUNT);

    /*
     * Two tokens produce exactly one unique token pair.
     */
    assert(result.scores.token_count == TOKEN_COUNT);
    assert(result.scores.token_pair_count == 1U);

    /*
     * With two samples for each of two subjects, comparing every
     * sample under token A with every sample under token B produces:
     *
     *     8 mated comparisons
     *     8 non-mated comparisons
     */
    assert(result.scores.mated_comparisons == 8U);
    assert(result.scores.non_mated_comparisons == 8U);

    assert(result.scores.mated_scores != NULL);
    assert(result.scores.non_mated_scores != NULL);

    /*
     * Normalized Hamming distances must remain in [0, 1].
     */
    assert(
        result.scores.minimum_mated_distance >= 0.0 &&
        result.scores.minimum_mated_distance <= 1.0
    );

    assert(
        result.scores.mean_mated_distance >= 0.0 &&
        result.scores.mean_mated_distance <= 1.0
    );

    assert(
        result.scores.maximum_mated_distance >= 0.0 &&
        result.scores.maximum_mated_distance <= 1.0
    );

    assert(
        result.scores.minimum_non_mated_distance >= 0.0 &&
        result.scores.minimum_non_mated_distance <= 1.0
    );

    assert(
        result.scores.mean_non_mated_distance >= 0.0 &&
        result.scores.mean_non_mated_distance <= 1.0
    );

    assert(
        result.scores.maximum_non_mated_distance >= 0.0 &&
        result.scores.maximum_non_mated_distance <= 1.0
    );

    /*
     * Verify that the score distributions were passed through to
     * the quantitative unlinkability stage.
     */
    assert(result.metric.bin_count == BIN_COUNT);
    assert(result.metric.omega == 1.0);

    assert(result.metric.bin_centers != NULL);
    assert(result.metric.local_unlinkability != NULL);
    assert(result.metric.mated_probabilities != NULL);
    assert(result.metric.non_mated_probabilities != NULL);

    /*
     * D_sys is defined over [0, 1].
     *
     * We deliberately do not assert a particular "good" value here.
     * This test validates implementation correctness, not biometric
     * performance of this tiny synthetic data set.
     */
    assert(isfinite(result.metric.global_unlinkability));

    assert(
        result.metric.global_unlinkability >= 0.0 &&
        result.metric.global_unlinkability <= 1.0
    );

    unlinkability_experiment_result_free(&result);

    /*
     * The public cleanup routine should release both nested result
     * structures and return the object to a zero-initialized state.
     */
    assert(result.sample_count == 0U);
    assert(result.feature_count == 0U);
    assert(result.hash_length == 0U);
    assert(result.token_count == 0U);

    assert(result.scores.mated_scores == NULL);
    assert(result.scores.non_mated_scores == NULL);

    assert(result.metric.bin_centers == NULL);
    assert(result.metric.local_unlinkability == NULL);
    assert(result.metric.mated_probabilities == NULL);
    assert(result.metric.non_mated_probabilities == NULL);
}

static void test_invalid_arguments_are_rejected(void)
{
    const BioHashConfig configs[TOKEN_COUNT] = {
        token_config(101U),
        token_config(103U)
    };

    UnlinkabilityExperimentResult result = {0};

    assert(
        run_unlinkability_experiment(
            NULL,
            SUBJECT_IDS,
            SAMPLE_COUNT,
            FEATURE_COUNT,
            HASH_LENGTH,
            configs,
            TOKEN_COUNT,
            BIN_COUNT,
            1.0,
            &result
        ) == UNLINKABILITY_EXPERIMENT_INVALID_ARGUMENT
    );

    assert(
        run_unlinkability_experiment(
            &FEATURES[0][0],
            SUBJECT_IDS,
            SAMPLE_COUNT,
            FEATURE_COUNT,
            HASH_LENGTH,
            configs,
            1U,
            BIN_COUNT,
            1.0,
            &result
        ) == UNLINKABILITY_EXPERIMENT_INVALID_ARGUMENT
    );

    assert(
        run_unlinkability_experiment(
            &FEATURES[0][0],
            SUBJECT_IDS,
            SAMPLE_COUNT,
            FEATURE_COUNT,
            FEATURE_COUNT + 1U,
            configs,
            TOKEN_COUNT,
            BIN_COUNT,
            1.0,
            &result
        ) == UNLINKABILITY_EXPERIMENT_INVALID_ARGUMENT
    );

    assert(
        run_unlinkability_experiment(
            &FEATURES[0][0],
            SUBJECT_IDS,
            SAMPLE_COUNT,
            FEATURE_COUNT,
            HASH_LENGTH,
            configs,
            TOKEN_COUNT,
            0U,
            1.0,
            &result
        ) == UNLINKABILITY_EXPERIMENT_INVALID_ARGUMENT
    );

    assert(
        run_unlinkability_experiment(
            &FEATURES[0][0],
            SUBJECT_IDS,
            SAMPLE_COUNT,
            FEATURE_COUNT,
            HASH_LENGTH,
            configs,
            TOKEN_COUNT,
            BIN_COUNT,
            0.0,
            &result
        ) == UNLINKABILITY_EXPERIMENT_INVALID_ARGUMENT
    );

    assert(
        run_unlinkability_experiment(
            &FEATURES[0][0],
            SUBJECT_IDS,
            SAMPLE_COUNT,
            FEATURE_COUNT,
            HASH_LENGTH,
            configs,
            TOKEN_COUNT,
            BIN_COUNT,
            NAN,
            &result
        ) == UNLINKABILITY_EXPERIMENT_INVALID_ARGUMENT
    );

    assert(
        run_unlinkability_experiment(
            &FEATURES[0][0],
            SUBJECT_IDS,
            SAMPLE_COUNT,
            FEATURE_COUNT,
            HASH_LENGTH,
            configs,
            TOKEN_COUNT,
            BIN_COUNT,
            1.0,
            NULL
        ) == UNLINKABILITY_EXPERIMENT_INVALID_ARGUMENT
    );
}

static void test_score_generation_failure_is_propagated(void)
{
    BioHashConfig configs[TOKEN_COUNT] = {
        token_config(101U),
        token_config(103U)
    };

    /*
     * 13 is prime but 13 mod 4 = 1, so it is not a valid
     * Blum prime. The underlying BioHash generation should therefore
     * fail, and the integration layer should report a score-generation
     * failure rather than a metric failure.
     */
    configs[1].p = 13U;

    UnlinkabilityExperimentResult result = {0};

    assert(
        run_unlinkability_experiment(
            &FEATURES[0][0],
            SUBJECT_IDS,
            SAMPLE_COUNT,
            FEATURE_COUNT,
            HASH_LENGTH,
            configs,
            TOKEN_COUNT,
            BIN_COUNT,
            1.0,
            &result
        ) ==
        UNLINKABILITY_EXPERIMENT_SCORE_GENERATION_FAILURE
    );

    assert(result.scores.mated_scores == NULL);
    assert(result.scores.non_mated_scores == NULL);
    assert(result.metric.bin_centers == NULL);
    assert(result.metric.local_unlinkability == NULL);
}

static void test_result_free_is_safe(void)
{
    unlinkability_experiment_result_free(NULL);

    UnlinkabilityExperimentResult result = {0};

    unlinkability_experiment_result_free(&result);

    assert(result.scores.mated_scores == NULL);
    assert(result.scores.non_mated_scores == NULL);

    assert(result.metric.bin_centers == NULL);
    assert(result.metric.local_unlinkability == NULL);
    assert(result.metric.mated_probabilities == NULL);
    assert(result.metric.non_mated_probabilities == NULL);
}

int main(void)
{
    test_end_to_end_unlinkability_experiment_runs();
    test_invalid_arguments_are_rejected();
    test_score_generation_failure_is_propagated();
    test_result_free_is_safe();

    printf(
        "All end-to-end unlinkability experiment tests passed.\n"
    );

    return 0;
}
