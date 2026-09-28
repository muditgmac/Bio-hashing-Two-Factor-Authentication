#include "unlinkability_metric.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>

#define TOLERANCE 1e-12

static void assert_close(
    double actual,
    double expected
)
{
    assert(fabs(actual - expected) < TOLERANCE);
}

static void test_identical_distributions_are_unlinkable(void)
{
    const double mated[] = {
        0.10,
        0.20,
        0.70,
        0.90
    };

    const double non_mated[] = {
        0.10,
        0.20,
        0.70,
        0.90
    };

    UnlinkabilityMetricResult result = {0};

    assert(
        evaluate_unlinkability_metric(
            mated,
            4U,
            non_mated,
            4U,
            10U,
            1.0,
            &result
        ) == UNLINKABILITY_METRIC_OK
    );

    assert_close(
        result.global_unlinkability,
        0.0
    );

    for (size_t bin = 0U; bin < result.bin_count; ++bin) {
        assert_close(
            result.local_unlinkability[bin],
            0.0
        );
    }

    unlinkability_metric_result_free(&result);
}

static void test_perfectly_separated_distributions_are_linkable(void)
{
    const double mated[] = {
        0.10,
        0.20
    };

    const double non_mated[] = {
        0.80,
        0.90
    };

    UnlinkabilityMetricResult result = {0};

    assert(
        evaluate_unlinkability_metric(
            mated,
            2U,
            non_mated,
            2U,
            10U,
            1.0,
            &result
        ) == UNLINKABILITY_METRIC_OK
    );

    assert_close(
        result.global_unlinkability,
        1.0
    );

    unlinkability_metric_result_free(&result);
}

static void test_partial_overlap_has_expected_metric(void)
{
    /*
     * With two bins:
     *
     * mated     = [0.5, 0.5]
     * non-mated = [0.25, 0.75]
     *
     * First-bin LR = 2:
     *
     * D = (2 - 1) / (2 + 1) = 1/3
     *
     * Second-bin LR = 2/3, therefore D = 0.
     *
     * D_sys = 0.5 * (1/3) = 1/6.
     */
    const double mated[] = {
        0.10,
        0.10,
        0.80,
        0.80
    };

    const double non_mated[] = {
        0.10,
        0.80,
        0.80,
        0.80
    };

    UnlinkabilityMetricResult result = {0};

    assert(
        evaluate_unlinkability_metric(
            mated,
            4U,
            non_mated,
            4U,
            2U,
            1.0,
            &result
        ) == UNLINKABILITY_METRIC_OK
    );

    assert_close(
        result.local_unlinkability[0],
        1.0 / 3.0
    );

    assert_close(
        result.local_unlinkability[1],
        0.0
    );

    assert_close(
        result.global_unlinkability,
        1.0 / 6.0
    );

    unlinkability_metric_result_free(&result);
}

static void test_omega_changes_prior_assumption(void)
{
    const double mated[] = {
        0.10,
        0.10,
        0.80,
        0.80
    };

    const double non_mated[] = {
        0.10,
        0.80,
        0.80,
        0.80
    };

    UnlinkabilityMetricResult result = {0};

    assert(
        evaluate_unlinkability_metric(
            mated,
            4U,
            non_mated,
            4U,
            2U,
            0.5,
            &result
        ) == UNLINKABILITY_METRIC_OK
    );

    /*
     * First-bin LR = 2 and omega = 0.5,
     * hence omega * LR = 1 and D = 0.
     */
    assert_close(
        result.global_unlinkability,
        0.0
    );

    unlinkability_metric_result_free(&result);
}

static void test_invalid_scores_are_rejected(void)
{
    const double valid[] = {
        0.10,
        0.20
    };

    const double negative[] = {
        -0.10,
        0.20
    };

    const double too_large[] = {
        0.10,
        1.10
    };

    const double nonfinite[] = {
        0.10,
        NAN
    };

    UnlinkabilityMetricResult result = {0};

    assert(
        evaluate_unlinkability_metric(
            negative,
            2U,
            valid,
            2U,
            10U,
            1.0,
            &result
        ) == UNLINKABILITY_METRIC_INVALID_SCORE
    );

    assert(
        evaluate_unlinkability_metric(
            valid,
            2U,
            too_large,
            2U,
            10U,
            1.0,
            &result
        ) == UNLINKABILITY_METRIC_INVALID_SCORE
    );

    assert(
        evaluate_unlinkability_metric(
            nonfinite,
            2U,
            valid,
            2U,
            10U,
            1.0,
            &result
        ) == UNLINKABILITY_METRIC_INVALID_SCORE
    );
}

static void test_invalid_arguments_are_rejected(void)
{
    const double scores[] = {
        0.10,
        0.20
    };

    UnlinkabilityMetricResult result = {0};

    assert(
        evaluate_unlinkability_metric(
            NULL,
            2U,
            scores,
            2U,
            10U,
            1.0,
            &result
        ) == UNLINKABILITY_METRIC_INVALID_ARGUMENT
    );

    assert(
        evaluate_unlinkability_metric(
            scores,
            0U,
            scores,
            2U,
            10U,
            1.0,
            &result
        ) == UNLINKABILITY_METRIC_INVALID_ARGUMENT
    );

    assert(
        evaluate_unlinkability_metric(
            scores,
            2U,
            scores,
            2U,
            0U,
            1.0,
            &result
        ) == UNLINKABILITY_METRIC_INVALID_ARGUMENT
    );

    assert(
        evaluate_unlinkability_metric(
            scores,
            2U,
            scores,
            2U,
            10U,
            0.0,
            &result
        ) == UNLINKABILITY_METRIC_INVALID_ARGUMENT
    );

    assert(
        evaluate_unlinkability_metric(
            scores,
            2U,
            scores,
            2U,
            10U,
            NAN,
            &result
        ) == UNLINKABILITY_METRIC_INVALID_ARGUMENT
    );
}

static void test_result_free_is_safe(void)
{
    unlinkability_metric_result_free(NULL);

    UnlinkabilityMetricResult result = {0};

    unlinkability_metric_result_free(&result);

    assert(result.bin_centers == NULL);
    assert(result.local_unlinkability == NULL);
    assert(result.mated_probabilities == NULL);
    assert(result.non_mated_probabilities == NULL);
}

int main(void)
{
    test_identical_distributions_are_unlinkable();
    test_perfectly_separated_distributions_are_linkable();
    test_partial_overlap_has_expected_metric();
    test_omega_changes_prior_assumption();
    test_invalid_scores_are_rejected();
    test_invalid_arguments_are_rejected();
    test_result_free_is_safe();

    printf(
        "All quantitative unlinkability metric tests passed.\n"
    );

    return 0;
}
