#include "unlinkability.h"

#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>

#define SAMPLE_COUNT 4U
#define FEATURE_COUNT 8U
#define HASH_LENGTH 4U
#define TOKEN_COUNT 3U

static const double FEATURES[SAMPLE_COUNT][FEATURE_COUNT] = {
    {
        1.0, 2.0, 3.0, 4.0,
        5.0, 6.0, 7.0, 8.0
    },
    {
        1.1, 1.9, 3.2, 3.9,
        5.1, 5.8, 7.2, 7.9
    },
    {
        -1.0, -2.0, -3.0, -4.0,
        -5.0, -6.0, -7.0, -8.0
    },
    {
        -1.2, -1.8, -3.1, -4.2,
        -4.9, -6.1, -6.8, -8.2
    }
};

static const size_t SUBJECT_IDS[SAMPLE_COUNT] = {
    10U,
    10U,
    20U,
    20U
};

static BioHashConfig token_config(uint64_t seed)
{
    BioHashConfig config = {
        .p = 499U,
        .q = 547U,
        .seed = seed,
        .threshold = 0.0,
        .orthogonality_tolerance = 1e-12
    };

    return config;
}

static int nearly_equal(
    double first,
    double second
)
{
    return fabs(first - second) < 1e-12;
}

static void verify_distribution_statistics(
    const double *scores,
    size_t count,
    double expected_minimum,
    double expected_mean,
    double expected_maximum
)
{
    assert(scores != NULL);
    assert(count > 0U);

    double minimum = 1.0;
    double maximum = 0.0;
    double sum = 0.0;

    for (size_t index = 0U; index < count; ++index) {
        const double score = scores[index];

        assert(isfinite(score));
        assert(score >= 0.0);
        assert(score <= 1.0);

        if (score < minimum) {
            minimum = score;
        }

        if (score > maximum) {
            maximum = score;
        }

        sum += score;
    }

    const double mean = sum / (double)count;

    assert(nearly_equal(minimum, expected_minimum));
    assert(nearly_equal(mean, expected_mean));
    assert(nearly_equal(maximum, expected_maximum));
}

static void test_cross_token_score_generation(void)
{
    const BioHashConfig configs[TOKEN_COUNT] = {
        token_config(3U),
        token_config(5U),
        token_config(6U)
    };

    UnlinkabilityResult result = {0};

    assert(
        evaluate_unlinkability_scores(
            &FEATURES[0][0],
            SUBJECT_IDS,
            SAMPLE_COUNT,
            FEATURE_COUNT,
            HASH_LENGTH,
            configs,
            TOKEN_COUNT,
            &result
        ) == UNLINKABILITY_OK
    );

    /*
     * Three tokens produce:
     *
     *     C(3, 2) = 3
     *
     * unique cross-token pairs.
     */
    assert(result.token_count == 3U);
    assert(result.token_pair_count == 3U);

    /*
     * For each token pair there are 4 x 4 = 16
     * cross-token sample comparisons.
     *
     * We have two subjects with two samples each.
     *
     * Mated comparisons per token pair:
     *
     *     (2 x 2) + (2 x 2) = 8
     *
     * Non-mated comparisons per token pair:
     *
     *     16 - 8 = 8
     *
     * Across three token pairs:
     *
     *     mated     = 3 x 8 = 24
     *     non-mated = 3 x 8 = 24
     */
    assert(result.mated_comparisons == 24U);
    assert(result.non_mated_comparisons == 24U);

    assert(result.mated_scores != NULL);
    assert(result.non_mated_scores != NULL);

    verify_distribution_statistics(
        result.mated_scores,
        result.mated_comparisons,
        result.minimum_mated_distance,
        result.mean_mated_distance,
        result.maximum_mated_distance
    );

    verify_distribution_statistics(
        result.non_mated_scores,
        result.non_mated_comparisons,
        result.minimum_non_mated_distance,
        result.mean_non_mated_distance,
        result.maximum_non_mated_distance
    );

    unlinkability_result_free(&result);

    assert(result.token_count == 0U);
    assert(result.token_pair_count == 0U);
    assert(result.mated_comparisons == 0U);
    assert(result.non_mated_comparisons == 0U);
    assert(result.mated_scores == NULL);
    assert(result.non_mated_scores == NULL);
}

static void test_two_tokens_produce_one_token_pair(void)
{
    const BioHashConfig configs[2U] = {
        token_config(3U),
        token_config(5U)
    };

    UnlinkabilityResult result = {0};

    assert(
        evaluate_unlinkability_scores(
            &FEATURES[0][0],
            SUBJECT_IDS,
            SAMPLE_COUNT,
            FEATURE_COUNT,
            HASH_LENGTH,
            configs,
            2U,
            &result
        ) == UNLINKABILITY_OK
    );

    assert(result.token_pair_count == 1U);
    assert(result.mated_comparisons == 8U);
    assert(result.non_mated_comparisons == 8U);

    unlinkability_result_free(&result);
}

static void test_all_same_subject_is_rejected(void)
{
    const size_t same_subject_ids[SAMPLE_COUNT] = {
        42U,
        42U,
        42U,
        42U
    };

    const BioHashConfig configs[2U] = {
        token_config(3U),
        token_config(5U)
    };

    UnlinkabilityResult result = {0};

    assert(
        evaluate_unlinkability_scores(
            &FEATURES[0][0],
            same_subject_ids,
            SAMPLE_COUNT,
            FEATURE_COUNT,
            HASH_LENGTH,
            configs,
            2U,
            &result
        ) == UNLINKABILITY_INSUFFICIENT_COMPARISONS
    );

    assert(result.mated_scores == NULL);
    assert(result.non_mated_scores == NULL);
}

static void test_invalid_token_configuration_is_reported(void)
{
    BioHashConfig configs[2U] = {
        token_config(3U),
        token_config(5U)
    };

    /*
     * 13 is prime, but:
     *
     *     13 mod 4 = 1
     *
     * so it is not a valid Blum prime.
     */
    configs[1].p = 13U;

    UnlinkabilityResult result = {0};

    assert(
        evaluate_unlinkability_scores(
            &FEATURES[0][0],
            SUBJECT_IDS,
            SAMPLE_COUNT,
            FEATURE_COUNT,
            HASH_LENGTH,
            configs,
            2U,
            &result
        ) == UNLINKABILITY_BIOHASH_FAILURE
    );

    assert(result.mated_scores == NULL);
    assert(result.non_mated_scores == NULL);
}

static void test_invalid_arguments_are_rejected(void)
{
    const BioHashConfig configs[2U] = {
        token_config(3U),
        token_config(5U)
    };

    UnlinkabilityResult result = {0};

    assert(
        evaluate_unlinkability_scores(
            NULL,
            SUBJECT_IDS,
            SAMPLE_COUNT,
            FEATURE_COUNT,
            HASH_LENGTH,
            configs,
            2U,
            &result
        ) == UNLINKABILITY_INVALID_ARGUMENT
    );

    assert(
        evaluate_unlinkability_scores(
            &FEATURES[0][0],
            NULL,
            SAMPLE_COUNT,
            FEATURE_COUNT,
            HASH_LENGTH,
            configs,
            2U,
            &result
        ) == UNLINKABILITY_INVALID_ARGUMENT
    );

    assert(
        evaluate_unlinkability_scores(
            &FEATURES[0][0],
            SUBJECT_IDS,
            SAMPLE_COUNT,
            FEATURE_COUNT,
            HASH_LENGTH,
            NULL,
            2U,
            &result
        ) == UNLINKABILITY_INVALID_ARGUMENT
    );

    assert(
        evaluate_unlinkability_scores(
            &FEATURES[0][0],
            SUBJECT_IDS,
            SAMPLE_COUNT,
            FEATURE_COUNT,
            HASH_LENGTH,
            configs,
            1U,
            &result
        ) == UNLINKABILITY_INVALID_ARGUMENT
    );

    assert(
        evaluate_unlinkability_scores(
            &FEATURES[0][0],
            SUBJECT_IDS,
            SAMPLE_COUNT,
            FEATURE_COUNT,
            FEATURE_COUNT + 1U,
            configs,
            2U,
            &result
        ) == UNLINKABILITY_INVALID_ARGUMENT
    );

    assert(
        evaluate_unlinkability_scores(
            &FEATURES[0][0],
            SUBJECT_IDS,
            SAMPLE_COUNT,
            FEATURE_COUNT,
            HASH_LENGTH,
            configs,
            2U,
            NULL
        ) == UNLINKABILITY_INVALID_ARGUMENT
    );
}

static void test_result_free_accepts_null(void)
{
    unlinkability_result_free(NULL);

    UnlinkabilityResult result = {0};

    unlinkability_result_free(&result);

    assert(result.mated_scores == NULL);
    assert(result.non_mated_scores == NULL);
}

int main(void)
{
    test_cross_token_score_generation();
    test_two_tokens_produce_one_token_pair();
    test_all_same_subject_is_rejected();
    test_invalid_token_configuration_is_reported();
    test_invalid_arguments_are_rejected();
    test_result_free_accepts_null();

    printf("All BioHash unlinkability score tests passed.\n");

    return 0;
}
