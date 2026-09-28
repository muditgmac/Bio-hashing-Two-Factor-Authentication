#include <assert.h>
#include <math.h>
#include <stdio.h>

#include "revocability.h"

#define FEATURE_COUNT 8U
#define HASH_LENGTH 4U
#define TOKEN_COUNT 3U

static const double FEATURES[FEATURE_COUNT] = {
    0.25,
    -0.75,
    1.50,
    0.50,
    -1.25,
    2.00,
    0.80,
    -0.40
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

static void test_revocability_experiment_runs(void)
{
    const BioHashConfig configs[TOKEN_COUNT] = {
        token_config(101U),
        token_config(103U),
        token_config(107U)
    };

    RevocabilityResult result;

    const RevocabilityStatus status =
        evaluate_revocability(
            FEATURES,
            FEATURE_COUNT,
            HASH_LENGTH,
            configs,
            TOKEN_COUNT,
            &result
        );

    assert(status == REVOCABILITY_OK);

    assert(result.token_count == TOKEN_COUNT);

    /*
     * For three token configurations:
     *
     *     C(3, 2) = 3
     *
     * unique cross-token comparisons.
     */
    assert(result.pairwise_comparisons == 3U);

    assert(result.identical_template_pairs <= 3U);

    assert(result.minimum_cross_token_distance >= 0.0);
    assert(result.minimum_cross_token_distance <= 1.0);

    assert(result.mean_cross_token_distance >= 0.0);
    assert(result.mean_cross_token_distance <= 1.0);

    assert(result.maximum_cross_token_distance >= 0.0);
    assert(result.maximum_cross_token_distance <= 1.0);

    assert(
        result.minimum_cross_token_distance <=
        result.mean_cross_token_distance
    );

    assert(
        result.mean_cross_token_distance <=
        result.maximum_cross_token_distance
    );
}

static void test_two_tokens_produce_one_comparison(void)
{
    const BioHashConfig configs[2U] = {
        token_config(101U),
        token_config(103U)
    };

    RevocabilityResult result;

    assert(
        evaluate_revocability(
            FEATURES,
            FEATURE_COUNT,
            HASH_LENGTH,
            configs,
            2U,
            &result
        ) == REVOCABILITY_OK
    );

    assert(result.token_count == 2U);
    assert(result.pairwise_comparisons == 1U);
}

static void test_same_configuration_produces_identical_templates(void)
{
    const BioHashConfig config =
        token_config(101U);

    const BioHashConfig configs[2U] = {
        config,
        config
    };

    RevocabilityResult result;

    assert(
        evaluate_revocability(
            FEATURES,
            FEATURE_COUNT,
            HASH_LENGTH,
            configs,
            2U,
            &result
        ) == REVOCABILITY_OK
    );

    /*
     * Deterministic BioHash generation under the same token must
     * produce the same protected template.
     */
    assert(result.pairwise_comparisons == 1U);
    assert(result.identical_template_pairs == 1U);

    assert(result.minimum_cross_token_distance == 0.0);
    assert(result.mean_cross_token_distance == 0.0);
    assert(result.maximum_cross_token_distance == 0.0);
}

static void test_invalid_arguments_are_rejected(void)
{
    const BioHashConfig configs[2U] = {
        token_config(101U),
        token_config(103U)
    };

    RevocabilityResult result;

    assert(
        evaluate_revocability(
            NULL,
            FEATURE_COUNT,
            HASH_LENGTH,
            configs,
            2U,
            &result
        ) == REVOCABILITY_INVALID_ARGUMENT
    );

    assert(
        evaluate_revocability(
            FEATURES,
            0U,
            HASH_LENGTH,
            configs,
            2U,
            &result
        ) == REVOCABILITY_INVALID_ARGUMENT
    );

    assert(
        evaluate_revocability(
            FEATURES,
            FEATURE_COUNT,
            0U,
            configs,
            2U,
            &result
        ) == REVOCABILITY_INVALID_ARGUMENT
    );

    assert(
        evaluate_revocability(
            FEATURES,
            FEATURE_COUNT,
            FEATURE_COUNT + 1U,
            configs,
            2U,
            &result
        ) == REVOCABILITY_INVALID_ARGUMENT
    );

    assert(
        evaluate_revocability(
            FEATURES,
            FEATURE_COUNT,
            HASH_LENGTH,
            NULL,
            2U,
            &result
        ) == REVOCABILITY_INVALID_ARGUMENT
    );

    assert(
        evaluate_revocability(
            FEATURES,
            FEATURE_COUNT,
            HASH_LENGTH,
            configs,
            1U,
            &result
        ) == REVOCABILITY_INVALID_ARGUMENT
    );

    assert(
        evaluate_revocability(
            FEATURES,
            FEATURE_COUNT,
            HASH_LENGTH,
            configs,
            2U,
            NULL
        ) == REVOCABILITY_INVALID_ARGUMENT
    );
}

static void test_invalid_token_configuration_is_reported(void)
{
    BioHashConfig configs[2U] = {
        token_config(101U),
        token_config(103U)
    };

    /*
     * 13 is prime, but 13 mod 4 = 1, so it is not a Blum prime.
     */
    configs[1].p = 13U;

    RevocabilityResult result;

    assert(
        evaluate_revocability(
            FEATURES,
            FEATURE_COUNT,
            HASH_LENGTH,
            configs,
            2U,
            &result
        ) == REVOCABILITY_BIOHASH_FAILURE
    );
}

int main(void)
{
    test_revocability_experiment_runs();
    test_two_tokens_produce_one_comparison();
    test_same_configuration_produces_identical_templates();
    test_invalid_arguments_are_rejected();
    test_invalid_token_configuration_is_reported();

    printf("All BioHash revocability tests passed.\n");

    return 0;
}
