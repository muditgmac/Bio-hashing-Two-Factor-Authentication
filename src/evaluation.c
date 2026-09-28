#include "evaluation.h"

#include <math.h>
#include <stddef.h>

static int valid_score(double score)
{
    return isfinite(score) &&
           score >= 0.0 &&
           score <= 1.0;
}

static EvaluationStatus validate_scores(
    const double *scores,
    size_t count
)
{
    if (scores == NULL || count == 0U) {
        return EVALUATION_INVALID_ARGUMENT;
    }

    for (size_t index = 0U; index < count; ++index) {
        if (!valid_score(scores[index])) {
            return EVALUATION_INVALID_SCORE;
        }
    }

    return EVALUATION_OK;
}

EvaluationStatus evaluate_error_rates(
    const double *genuine_scores,
    size_t genuine_count,
    const double *impostor_scores,
    size_t impostor_count,
    double threshold,
    ErrorRates *result
)
{
    if (
        result == NULL ||
        !valid_score(threshold)
    ) {
        return EVALUATION_INVALID_ARGUMENT;
    }

    EvaluationStatus status =
        validate_scores(genuine_scores, genuine_count);

    if (status != EVALUATION_OK) {
        return status;
    }

    status =
        validate_scores(impostor_scores, impostor_count);

    if (status != EVALUATION_OK) {
        return status;
    }

    size_t false_matches = 0U;
    size_t false_non_matches = 0U;

    for (size_t index = 0U; index < impostor_count; ++index) {
        if (impostor_scores[index] <= threshold) {
            ++false_matches;
        }
    }

    for (size_t index = 0U; index < genuine_count; ++index) {
        if (genuine_scores[index] > threshold) {
            ++false_non_matches;
        }
    }

    result->threshold = threshold;

    result->fmr =
        (double)false_matches /
        (double)impostor_count;

    result->fnmr =
        (double)false_non_matches /
        (double)genuine_count;

    return EVALUATION_OK;
}

static void consider_candidate(
    const double *genuine_scores,
    size_t genuine_count,
    const double *impostor_scores,
    size_t impostor_count,
    double threshold,
    double *best_difference,
    EqualErrorRate *best_result
)
{
    ErrorRates rates;

    const EvaluationStatus status =
        evaluate_error_rates(
            genuine_scores,
            genuine_count,
            impostor_scores,
            impostor_count,
            threshold,
            &rates
        );

    if (status != EVALUATION_OK) {
        return;
    }

    const double difference =
        fabs(rates.fmr - rates.fnmr);

    if (difference < *best_difference) {
        *best_difference = difference;

        best_result->threshold = threshold;
        best_result->fmr = rates.fmr;
        best_result->fnmr = rates.fnmr;
        best_result->eer =
            (rates.fmr + rates.fnmr) / 2.0;
    }
}

EvaluationStatus estimate_equal_error_rate(
    const double *genuine_scores,
    size_t genuine_count,
    const double *impostor_scores,
    size_t impostor_count,
    EqualErrorRate *result
)
{
    if (result == NULL) {
        return EVALUATION_INVALID_ARGUMENT;
    }

    EvaluationStatus status =
        validate_scores(genuine_scores, genuine_count);

    if (status != EVALUATION_OK) {
        return status;
    }

    status =
        validate_scores(impostor_scores, impostor_count);

    if (status != EVALUATION_OK) {
        return status;
    }

    double best_difference = INFINITY;

    /*
     * Include the boundaries because they are valid decision
     * thresholds for normalized Hamming distance.
     */
    consider_candidate(
        genuine_scores,
        genuine_count,
        impostor_scores,
        impostor_count,
        0.0,
        &best_difference,
        result
    );

    consider_candidate(
        genuine_scores,
        genuine_count,
        impostor_scores,
        impostor_count,
        1.0,
        &best_difference,
        result
    );

    /*
     * Error rates can only change when the threshold crosses an
     * observed comparison score, so all observed distances are
     * sufficient candidate operating points for this discrete
     * estimate.
     */
    for (size_t index = 0U; index < genuine_count; ++index) {
        consider_candidate(
            genuine_scores,
            genuine_count,
            impostor_scores,
            impostor_count,
            genuine_scores[index],
            &best_difference,
            result
        );
    }

    for (size_t index = 0U; index < impostor_count; ++index) {
        consider_candidate(
            genuine_scores,
            genuine_count,
            impostor_scores,
            impostor_count,
            impostor_scores[index],
            &best_difference,
            result
        );
    }

    return EVALUATION_OK;
}
