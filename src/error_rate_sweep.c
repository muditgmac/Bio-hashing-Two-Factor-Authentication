#include "error_rate_sweep.h"

#include <stdint.h>
#include <stdlib.h>

void error_rate_sweep_result_free(
    ErrorRateSweepResult *result
)
{
    if (result == NULL) {
        return;
    }

    free(result->points);

    result->points = NULL;
    result->point_count = 0U;
}

static ErrorRateSweepStatus map_evaluation_status(
    EvaluationStatus status
)
{
    switch (status) {
        case EVALUATION_OK:
            return ERROR_RATE_SWEEP_OK;

        case EVALUATION_INVALID_SCORE:
            return ERROR_RATE_SWEEP_INVALID_SCORE;

        case EVALUATION_INVALID_ARGUMENT:
            return ERROR_RATE_SWEEP_INVALID_ARGUMENT;
    }

    return ERROR_RATE_SWEEP_EVALUATION_FAILURE;
}

ErrorRateSweepStatus evaluate_error_rate_sweep(
    const double *genuine_scores,
    size_t genuine_count,
    const double *impostor_scores,
    size_t impostor_count,
    size_t hash_length,
    ErrorRateSweepResult *result
)
{
    if (
        genuine_scores == NULL ||
        genuine_count == 0U ||
        impostor_scores == NULL ||
        impostor_count == 0U ||
        hash_length == 0U ||
        result == NULL
    ) {
        return ERROR_RATE_SWEEP_INVALID_ARGUMENT;
    }

    if (hash_length == SIZE_MAX) {
        return ERROR_RATE_SWEEP_INVALID_ARGUMENT;
    }

    const size_t point_count =
        hash_length + 1U;

    if (
        point_count >
        SIZE_MAX / sizeof(ErrorRates)
    ) {
        return ERROR_RATE_SWEEP_INVALID_ARGUMENT;
    }

    ErrorRates *points =
        malloc(point_count * sizeof(*points));

    if (points == NULL) {
        return ERROR_RATE_SWEEP_ALLOCATION_FAILURE;
    }

    for (
        size_t step = 0U;
        step < point_count;
        ++step
    ) {
        const double threshold =
            (double)step /
            (double)hash_length;

        const EvaluationStatus evaluation_status =
            evaluate_error_rates(
                genuine_scores,
                genuine_count,
                impostor_scores,
                impostor_count,
                threshold,
                &points[step]
            );

        if (evaluation_status != EVALUATION_OK) {
            const ErrorRateSweepStatus status =
                map_evaluation_status(
                    evaluation_status
                );

            free(points);

            return status;
        }
    }

    const ErrorRateSweepResult generated = {
        .point_count = point_count,
        .points = points
    };

    *result = generated;

    return ERROR_RATE_SWEEP_OK;
}
