#include "matcher.h"

#include <assert.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define TEST_TOLERANCE 1e-12

static void test_identical_templates_have_zero_distance(void)
{
    const uint8_t first[] = {1U, 0U, 1U, 1U, 0U, 0U};
    const uint8_t second[] = {1U, 0U, 1U, 1U, 0U, 0U};

    size_t distance = 99U;

    assert(
        hamming_distance(
            first,
            second,
            6U,
            &distance
        ) == MATCHER_OK
    );

    assert(distance == 0U);
}

static void test_completely_different_templates(void)
{
    const uint8_t first[] = {0U, 0U, 0U, 0U};
    const uint8_t second[] = {1U, 1U, 1U, 1U};

    size_t distance = 0U;

    assert(
        hamming_distance(
            first,
            second,
            4U,
            &distance
        ) == MATCHER_OK
    );

    assert(distance == 4U);
}

static void test_partial_hamming_distance(void)
{
    const uint8_t first[] = {
        1U, 0U, 1U, 0U,
        1U, 1U, 0U, 0U
    };

    const uint8_t second[] = {
        1U, 1U, 1U, 0U,
        0U, 1U, 0U, 1U
    };

    size_t distance = 0U;

    assert(
        hamming_distance(
            first,
            second,
            8U,
            &distance
        ) == MATCHER_OK
    );

    assert(distance == 3U);
}

static void test_normalized_distance(void)
{
    const uint8_t first[] = {1U, 0U, 1U, 0U};
    const uint8_t second[] = {1U, 1U, 0U, 0U};

    double distance = -1.0;

    assert(
        normalized_hamming_distance(
            first,
            second,
            4U,
            &distance
        ) == MATCHER_OK
    );

    assert(fabs(distance - 0.5) < TEST_TOLERANCE);
}

static void test_threshold_accepts_close_templates(void)
{
    const uint8_t first[] = {1U, 0U, 1U, 0U};
    const uint8_t second[] = {1U, 0U, 1U, 1U};

    int is_match = 0;

    assert(
        biohash_match(
            first,
            second,
            4U,
            0.25,
            &is_match
        ) == MATCHER_OK
    );

    assert(is_match == 1);
}

static void test_threshold_rejects_distant_templates(void)
{
    const uint8_t first[] = {1U, 0U, 1U, 0U};
    const uint8_t second[] = {0U, 1U, 1U, 1U};

    int is_match = 1;

    assert(
        biohash_match(
            first,
            second,
            4U,
            0.25,
            &is_match
        ) == MATCHER_OK
    );

    assert(is_match == 0);
}

static void test_threshold_boundary_is_inclusive(void)
{
    const uint8_t first[] = {1U, 0U, 1U, 0U};
    const uint8_t second[] = {1U, 0U, 1U, 1U};

    int is_match = 0;

    assert(
        biohash_match(
            first,
            second,
            4U,
            0.25,
            &is_match
        ) == MATCHER_OK
    );

    assert(is_match == 1);
}

static void test_invalid_template_value_is_rejected(void)
{
    const uint8_t first[] = {1U, 0U, 2U, 0U};
    const uint8_t second[] = {1U, 0U, 1U, 0U};

    size_t distance = 0U;

    assert(
        hamming_distance(
            first,
            second,
            4U,
            &distance
        ) == MATCHER_INVALID_TEMPLATE
    );
}

static void test_invalid_arguments_are_rejected(void)
{
    const uint8_t valid[] = {0U, 1U};

    size_t distance = 0U;
    double normalized = 0.0;
    int is_match = 0;

    assert(
        hamming_distance(
            NULL,
            valid,
            2U,
            &distance
        ) == MATCHER_INVALID_ARGUMENT
    );

    assert(
        hamming_distance(
            valid,
            valid,
            0U,
            &distance
        ) == MATCHER_INVALID_ARGUMENT
    );

    assert(
        normalized_hamming_distance(
            valid,
            valid,
            2U,
            NULL
        ) == MATCHER_INVALID_ARGUMENT
    );

    assert(
        biohash_match(
            valid,
            valid,
            2U,
            -0.01,
            &is_match
        ) == MATCHER_INVALID_ARGUMENT
    );

    assert(
        biohash_match(
            valid,
            valid,
            2U,
            1.01,
            &is_match
        ) == MATCHER_INVALID_ARGUMENT
    );

    assert(
        biohash_match(
            valid,
            valid,
            2U,
            NAN,
            &is_match
        ) == MATCHER_INVALID_ARGUMENT
    );

    (void)normalized;
}

int main(void)
{
    test_identical_templates_have_zero_distance();
    test_completely_different_templates();
    test_partial_hamming_distance();
    test_normalized_distance();
    test_threshold_accepts_close_templates();
    test_threshold_rejects_distant_templates();
    test_threshold_boundary_is_inclusive();
    test_invalid_template_value_is_rejected();
    test_invalid_arguments_are_rejected();

    printf("All BioHash matcher tests passed.\n");

    return 0;
}
