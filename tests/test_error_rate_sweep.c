#include "error_rate_sweep.h"

#include <assert.h>
#include <math.h>
#include <stddef.h>
#include <stdio.h>

#define CHECK_TOLERANCE 1e-12

static void assert_close(
    double actual,
    double expected
)
{
    assert(
        fabs(actual - expected) <
        CHECK_TOLERANCE
    );
}

static void test_four_bit_threshold_sweep(void)
{
    const double genuine_scores[] = {
        0.0,
        0.25,
        0.50
    };

    const double impostor_scores[] = {
        0.50,
        0.75,
        1.00
    };

    ErrorRateSweepResult result = {0};

    assert(
        evaluate_error_rate_sweep(
            genuine_scores,
            3U,
            impostor_scores,
            3U,
            4U,
            &result
        ) == ERROR_RATE_SWEEP_OK
    );

    assert(result.point_count == 5U);
    assert(result.points != NULL);

    /*
     * threshold = 0.00
     *
     * Genuine accepted: 0.00
     * Genuine rejected: 0.25, 0.50
     *
     * FMR  = 0 / 3
     * FNMR = 2 / 3
     */
    assert_close(result.points[0U].threshold, 0.00);
    assert_close(result.points[0U].fmr, 0.0);
    assert_close(result.points[0U].fnmr, 2.0 / 3.0);

    /*
     * threshold = 0.25
     */
    assert_close(result.points[1U].threshold, 0.25);
    assert_close(result.points[1U].fmr, 0.0);
    assert_close(result.points[1U].fnmr, 1.0 / 3.0);

    /*
     * threshold = 0.50
     *
     * The boundary is inclusive because distance <= threshold
     * is considered a match.
     */
    assert_close(result.points[2U].threshold, 0.50);
    assert_close(result.points[2U].fmr, 1.0 / 3.0);
    assert_close(result.points[2U].fnmr, 0.0);

    /*
     * threshold = 0.75
     */
    assert_close(result.points[3U].threshold, 0.75);
    assert_close(result.points[3U].fmr, 2.0 / 3.0);
    assert_close(result.points[3U].fnmr, 0.0);

    /*
     * threshold = 1.00
     */
    assert_close(result.points[4U].threshold, 1.00);
    assert_close(result.points[4U].fmr, 1.0);
    assert_close(result.points[4U].fnmr, 0.0);

    error_rate_sweep_result_free(&result);

    assert(result.points == NULL);
    assert(result.point_count == 0U);
}

static void test_three_bit_thresholds(void)
{
    const double genuine_scores[] = {
        0.0,
        1.0 / 3.0
    };

    const double impostor_scores[] = {
        2.0 / 3.0,
        1.0
    };

    ErrorRateSweepResult result = {0};

    assert(
        evaluate_error_rate_sweep(
            genuine_scores,
            2U,
            impostor_scores,
            2U,
            3U,
            &result
        ) == ERROR_RATE_SWEEP_OK
    );

    assert(result.point_count == 4U);

    assert_close(result.points[0U].threshold, 0.0);
    assert_close(
        result.points[1U].threshold,
        1.0 / 3.0
    );
    assert_close(
        result.points[2U].threshold,
        2.0 / 3.0
    );
    assert_close(result.points[3U].threshold, 1.0);

    error_rate_sweep_result_free(&result);
}

static void test_invalid_score_is_reported(void)
{
    const double genuine_scores[] = {
        0.0,
        1.25
    };

    const double impostor_scores[] = {
        0.5,
        0.75
    };

    ErrorRateSweepResult result = {0};

    assert(
        evaluate_error_rate_sweep(
            genuine_scores,
            2U,
            impostor_scores,
            2U,
            4U,
            &result
        ) == ERROR_RATE_SWEEP_INVALID_SCORE
    );

    assert(result.points == NULL);
    assert(result.point_count == 0U);
}

static void test_invalid_arguments_are_rejected(void)
{
    const double scores[] = {
        0.0,
        0.5
    };

    ErrorRateSweepResult result = {0};

    assert(
        evaluate_error_rate_sweep(
            NULL,
            2U,
            scores,
            2U,
            4U,
            &result
        ) == ERROR_RATE_SWEEP_INVALID_ARGUMENT
    );

    assert(
        evaluate_error_rate_sweep(
            scores,
            0U,
            scores,
            2U,
            4U,
            &result
        ) == ERROR_RATE_SWEEP_INVALID_ARGUMENT
    );

    assert(
        evaluate_error_rate_sweep(
            scores,
            2U,
            NULL,
            2U,
            4U,
            &result
        ) == ERROR_RATE_SWEEP_INVALID_ARGUMENT
    );

    assert(
        evaluate_error_rate_sweep(
            scores,
            2U,
            scores,
            0U,
            4U,
            &result
        ) == ERROR_RATE_SWEEP_INVALID_ARGUMENT
    );

    assert(
        evaluate_error_rate_sweep(
            scores,
            2U,
            scores,
            2U,
            0U,
            &result
        ) == ERROR_RATE_SWEEP_INVALID_ARGUMENT
    );

    assert(
        evaluate_error_rate_sweep(
            scores,
            2U,
            scores,
            2U,
            4U,
            NULL
        ) == ERROR_RATE_SWEEP_INVALID_ARGUMENT
    );
}

static void test_free_is_null_safe(void)
{
    error_rate_sweep_result_free(NULL);

    ErrorRateSweepResult result = {0};

    error_rate_sweep_result_free(&result);

    assert(result.points == NULL);
    assert(result.point_count == 0U);
}

int main(void)
{
    test_four_bit_threshold_sweep();
    test_three_bit_thresholds();
    test_invalid_score_is_reported();
    test_invalid_arguments_are_rejected();
    test_free_is_null_safe();

    printf(
        "All biometric error-rate sweep tests passed.\n"
    );

    return 0;
}
