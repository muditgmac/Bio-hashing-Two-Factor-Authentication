#include "biohash.h"

#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define FEATURE_COUNT 32U
#define HASH_LENGTH 8U

static const double TEST_FEATURES[FEATURE_COUNT] = {
     0.12, -0.31,  0.47,  0.83,
    -0.22,  0.64, -0.71,  0.35,
     0.91, -0.56,  0.18, -0.44,
     0.73,  0.27, -0.62,  0.51,
    -0.13,  0.39,  0.68, -0.84,
     0.24, -0.49,  0.77,  0.06,
    -0.95,  0.58,  0.33, -0.17,
     0.81, -0.28,  0.45, -0.66
};

static BioHashConfig default_config(void)
{
    BioHashConfig config = {
        .p = 499U,
        .q = 547U,
        .seed = 5U,
        .threshold = 0.0,
        .orthogonality_tolerance = 1e-12
    };

    return config;
}

static void assert_binary(
    const uint8_t *hash,
    size_t length
)
{
    for (size_t index = 0U; index < length; ++index) {
        assert(hash[index] == 0U || hash[index] == 1U);
    }
}

static void test_generation_succeeds(void)
{
    BioHashConfig config = default_config();
    uint8_t output[HASH_LENGTH];

    assert(
        biohash_generate(
            TEST_FEATURES,
            FEATURE_COUNT,
            HASH_LENGTH,
            &config,
            output
        ) == BIOHASH_OK
    );

    assert_binary(output, HASH_LENGTH);
}

static void test_same_token_is_deterministic(void)
{
    BioHashConfig config = default_config();

    uint8_t first[HASH_LENGTH];
    uint8_t second[HASH_LENGTH];

    assert(
        biohash_generate(
            TEST_FEATURES,
            FEATURE_COUNT,
            HASH_LENGTH,
            &config,
            first
        ) == BIOHASH_OK
    );

    assert(
        biohash_generate(
            TEST_FEATURES,
            FEATURE_COUNT,
            HASH_LENGTH,
            &config,
            second
        ) == BIOHASH_OK
    );

    assert(
        memcmp(
            first,
            second,
            sizeof(first)
        ) == 0
    );
}

static void test_different_token_can_change_template(void)
{
    BioHashConfig config = default_config();

    uint8_t baseline[HASH_LENGTH];
    uint8_t candidate[HASH_LENGTH];

    assert(
        biohash_generate(
            TEST_FEATURES,
            FEATURE_COUNT,
            HASH_LENGTH,
            &config,
            baseline
        ) == BIOHASH_OK
    );

    const uint64_t candidate_seeds[] = {
        7U,
        13U,
        17U,
        19U,
        23U,
        29U,
        31U,
        37U
    };

    int difference_found = 0;

    for (
        size_t index = 0U;
        index <
            sizeof(candidate_seeds) /
            sizeof(candidate_seeds[0]);
        ++index
    ) {
        config.seed = candidate_seeds[index];

        const BioHashStatus status =
            biohash_generate(
                TEST_FEATURES,
                FEATURE_COUNT,
                HASH_LENGTH,
                &config,
                candidate
            );

        if (
            status == BIOHASH_OK &&
            memcmp(
                baseline,
                candidate,
                sizeof(baseline)
            ) != 0
        ) {
            difference_found = 1;
            break;
        }
    }

    assert(difference_found);
}

static void test_high_threshold_produces_zero_template(void)
{
    BioHashConfig config = default_config();
    config.threshold = 1e9;

    uint8_t output[HASH_LENGTH];

    assert(
        biohash_generate(
            TEST_FEATURES,
            FEATURE_COUNT,
            HASH_LENGTH,
            &config,
            output
        ) == BIOHASH_OK
    );

    for (size_t index = 0U; index < HASH_LENGTH; ++index) {
        assert(output[index] == 0U);
    }
}

static void test_low_threshold_produces_one_template(void)
{
    BioHashConfig config = default_config();
    config.threshold = -1e9;

    uint8_t output[HASH_LENGTH];

    assert(
        biohash_generate(
            TEST_FEATURES,
            FEATURE_COUNT,
            HASH_LENGTH,
            &config,
            output
        ) == BIOHASH_OK
    );

    for (size_t index = 0U; index < HASH_LENGTH; ++index) {
        assert(output[index] == 1U);
    }
}

static void test_invalid_arguments_are_rejected(void)
{
    BioHashConfig config = default_config();
    uint8_t output[HASH_LENGTH];

    assert(
        biohash_generate(
            NULL,
            FEATURE_COUNT,
            HASH_LENGTH,
            &config,
            output
        ) == BIOHASH_INVALID_ARGUMENT
    );

    assert(
        biohash_generate(
            TEST_FEATURES,
            0U,
            HASH_LENGTH,
            &config,
            output
        ) == BIOHASH_INVALID_ARGUMENT
    );

    assert(
        biohash_generate(
            TEST_FEATURES,
            FEATURE_COUNT,
            0U,
            &config,
            output
        ) == BIOHASH_INVALID_ARGUMENT
    );

    assert(
        biohash_generate(
            TEST_FEATURES,
            FEATURE_COUNT,
            FEATURE_COUNT + 1U,
            &config,
            output
        ) == BIOHASH_INVALID_ARGUMENT
    );

    assert(
        biohash_generate(
            TEST_FEATURES,
            FEATURE_COUNT,
            HASH_LENGTH,
            NULL,
            output
        ) == BIOHASH_INVALID_ARGUMENT
    );

    assert(
        biohash_generate(
            TEST_FEATURES,
            FEATURE_COUNT,
            HASH_LENGTH,
            &config,
            NULL
        ) == BIOHASH_INVALID_ARGUMENT
    );
}

static void test_nonfinite_feature_is_rejected(void)
{
    BioHashConfig config = default_config();

    double features[FEATURE_COUNT];
    memcpy(
        features,
        TEST_FEATURES,
        sizeof(features)
    );

    features[7] = NAN;

    uint8_t output[HASH_LENGTH];

    assert(
        biohash_generate(
            features,
            FEATURE_COUNT,
            HASH_LENGTH,
            &config,
            output
        ) == BIOHASH_INVALID_ARGUMENT
    );
}

static void test_nonfinite_configuration_is_rejected(void)
{
    BioHashConfig config = default_config();
    uint8_t output[HASH_LENGTH];

    config.threshold = NAN;

    assert(
        biohash_generate(
            TEST_FEATURES,
            FEATURE_COUNT,
            HASH_LENGTH,
            &config,
            output
        ) == BIOHASH_INVALID_ARGUMENT
    );

    config = default_config();
    config.orthogonality_tolerance = INFINITY;

    assert(
        biohash_generate(
            TEST_FEATURES,
            FEATURE_COUNT,
            HASH_LENGTH,
            &config,
            output
        ) == BIOHASH_INVALID_ARGUMENT
    );

    config = default_config();
    config.orthogonality_tolerance = -1.0;

    assert(
        biohash_generate(
            TEST_FEATURES,
            FEATURE_COUNT,
            HASH_LENGTH,
            &config,
            output
        ) == BIOHASH_INVALID_ARGUMENT
    );
}

static void test_invalid_bbs_parameters_are_rejected(void)
{
    BioHashConfig config = default_config();

    /*
     * 13 is prime but is not congruent to 3 modulo 4,
     * so it is not a valid Blum prime.
     */
    config.p = 13U;

    uint8_t output[HASH_LENGTH];

    assert(
        biohash_generate(
            TEST_FEATURES,
            FEATURE_COUNT,
            HASH_LENGTH,
            &config,
            output
        ) == BIOHASH_BBS_FAILURE
    );
}

int main(void)
{
    test_generation_succeeds();
    test_same_token_is_deterministic();
    test_different_token_can_change_template();
    test_high_threshold_produces_zero_template();
    test_low_threshold_produces_one_template();
    test_invalid_arguments_are_rejected();
    test_nonfinite_feature_is_rejected();
    test_nonfinite_configuration_is_rejected();
    test_invalid_bbs_parameters_are_rejected();

    printf("All integrated BioHash tests passed.\n");

    return 0;
}
