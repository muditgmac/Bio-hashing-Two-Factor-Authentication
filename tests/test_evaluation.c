#include "evaluation.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>

#define EPSILON 1e-12

static int approximately_equal(double first, double second)
{
    return fabs(first - second) < EPSILON;
}

static void test_perfect_separation(void)
{
    const double genuine[] = {
        0.05,
        0.10,
        0.15
    };

    const double impostor[] = {
        0.70,
        0.80,
        0.90
    };

    ErrorRates result;

    assert(
        evaluate_error_rates(
            genuine,
            3U,
            impostor,
            3U,
            0.40,
            &result
        ) == EVALUATION_OK
    );

    assert(approximately_equal(result.fmr, 0.0));
    assert(approximately_equal(result.fnmr, 0.0));
}

static void test_false_match_rate(void)
{
    const double genuine[] = {
        0.10,
        0.20
    };

    const double impostor[] = {
        0.20,
        0.60,
        0.80,
        0.90
    };

    ErrorRates result;

    assert(
        evaluate_error_rates(
            genuine,
            2U,
            impostor,
            4U,
            0.25,
            &result
        ) == EVALUATION_OK
    );

    assert(
        approximately_equal(
            result.fmr,
            0.25
        )
    );

    assert(approximately_equal(result.fnmr, 0.0));
}

static void test_false_non_match_rate(void)
{
    const double genuine[] = {
        0.10,
        0.20,
        0.40,
        0.60
    };

    const double impostor[] = {
        0.70,
        0.80
    };

    ErrorRates result;

    assert(
        evaluate_error_rates(
            genuine,
            4U,
            impostor,
            2U,
            0.25,
            &result
        ) == EVALUATION_OK
    );

    assert(approximately_equal(result.fmr, 0.0));

    assert(
        approximately_equal(
            result.fnmr,
            0.50
        )
    );
}

static void test_threshold_boundary_is_inclusive(void)
{
    const double genuine[] = {
        0.30
    };

    const double impostor[] = {
        0.30
    };

    ErrorRates result;

    assert(
        evaluate_error_rates(
            genuine,
            1U,
            impostor,
            1U,
            0.30,
            &result
        ) == EVALUATION_OK
    );

    /*
     * distance <= threshold is considered a match.
     */
    assert(approximately_equal(result.fnmr, 0.0));
    assert(approximately_equal(result.fmr, 1.0));
}

static void test_eer_with_crossing(void)
{
    const double genuine[] = {
        0.10,
        0.20,
        0.40,
        0.60
    };

    const double impostor[] = {
        0.30,
        0.50,
        0.70,
        0.90
    };

    EqualErrorRate result;

    assert(
        estimate_equal_error_rate(
            genuine,
            4U,
            impostor,
            4U,
            &result
        ) == EVALUATION_OK
    );

    assert(
        approximately_equal(
            result.fmr,
            result.fnmr
        )
    );

    assert(
        approximately_equal(
            result.eer,
            0.25
        )
    );
}

static void test_perfect_system_has_zero_eer(void)
{
    const double genuine[] = {
        0.05,
        0.10,
        0.15
    };

    const double impostor[] = {
        0.70,
        0.80,
        0.90
    };

    EqualErrorRate result;

    assert(
        estimate_equal_error_rate(
            genuine,
            3U,
            impostor,
            3U,
            &result
        ) == EVALUATION_OK
    );

    assert(approximately_equal(result.eer, 0.0));
}

static void test_invalid_threshold(void)
{
    const double genuine[] = {0.10};
    const double impostor[] = {0.80};

    ErrorRates result;

    assert(
        evaluate_error_rates(
            genuine,
            1U,
            impostor,
            1U,
            -0.01,
            &result
        ) == EVALUATION_INVALID_ARGUMENT
    );

    assert(
        evaluate_error_rates(
            genuine,
            1U,
            impostor,
            1U,
            1.01,
            &result
        ) == EVALUATION_INVALID_ARGUMENT
    );

    assert(
        evaluate_error_rates(
            genuine,
            1U,
            impostor,
            1U,
            NAN,
            &result
        ) == EVALUATION_INVALID_ARGUMENT
    );
}

static void test_invalid_scores(void)
{
    const double genuine[] = {
        0.10,
        NAN
    };

    const double impostor[] = {
        0.80
    };

    ErrorRates result;

    assert(
        evaluate_error_rates(
            genuine,
            2U,
            impostor,
            1U,
            0.50,
            &result
        ) == EVALUATION_INVALID_SCORE
    );
}

static void test_empty_input_is_rejected(void)
{
    const double genuine[] = {0.10};
    const double impostor[] = {0.80};

    EqualErrorRate result;

    assert(
        estimate_equal_error_rate(
            genuine,
            0U,
            impostor,
            1U,
            &result
        ) == EVALUATION_INVALID_ARGUMENT
    );

    assert(
        estimate_equal_error_rate(
            genuine,
            1U,
            impostor,
            0U,
            &result
        ) == EVALUATION_INVALID_ARGUMENT
    );
}

int main(void)
{
    test_perfect_separation();
    test_false_match_rate();
    test_false_non_match_rate();
    test_threshold_boundary_is_inclusive();
    test_eer_with_crossing();
    test_perfect_system_has_zero_eer();
    test_invalid_threshold();
    test_invalid_scores();
    test_empty_input_is_rejected();

    printf("All biometric evaluation tests passed.\n");

    return 0;
}
