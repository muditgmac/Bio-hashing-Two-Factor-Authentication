#include "unlinkability_metric.h"

#include <math.h>
#include <stdlib.h>

static int score_is_valid(double score)
{
    return
        isfinite(score) &&
        score >= 0.0 &&
        score <= 1.0;
}

static size_t score_bin(
    double score,
    size_t bin_count
)
{
    if (score >= 1.0) {
        return bin_count - 1U;
    }

    return (size_t)(score * (double)bin_count);
}

void unlinkability_metric_result_free(
    UnlinkabilityMetricResult *result
)
{
    if (result == NULL) {
        return;
    }

    free(result->bin_centers);
    free(result->local_unlinkability);
    free(result->mated_probabilities);
    free(result->non_mated_probabilities);

    result->bin_count = 0U;
    result->omega = 0.0;
    result->global_unlinkability = 0.0;

    result->bin_centers = NULL;
    result->local_unlinkability = NULL;
    result->mated_probabilities = NULL;
    result->non_mated_probabilities = NULL;
}

UnlinkabilityMetricStatus evaluate_unlinkability_metric(
    const double *mated_scores,
    size_t mated_count,
    const double *non_mated_scores,
    size_t non_mated_count,
    size_t bin_count,
    double omega,
    UnlinkabilityMetricResult *result
)
{
    if (
        mated_scores == NULL ||
        non_mated_scores == NULL ||
        result == NULL ||
        mated_count == 0U ||
        non_mated_count == 0U ||
        bin_count == 0U ||
        !isfinite(omega) ||
        omega <= 0.0
    ) {
        return UNLINKABILITY_METRIC_INVALID_ARGUMENT;
    }

    for (size_t index = 0U; index < mated_count; ++index) {
        if (!score_is_valid(mated_scores[index])) {
            return UNLINKABILITY_METRIC_INVALID_SCORE;
        }
    }

    for (size_t index = 0U; index < non_mated_count; ++index) {
        if (!score_is_valid(non_mated_scores[index])) {
            return UNLINKABILITY_METRIC_INVALID_SCORE;
        }
    }

    double *bin_centers =
        calloc(bin_count, sizeof(*bin_centers));

    double *local_unlinkability =
        calloc(bin_count, sizeof(*local_unlinkability));

    double *mated_probabilities =
        calloc(bin_count, sizeof(*mated_probabilities));

    double *non_mated_probabilities =
        calloc(bin_count, sizeof(*non_mated_probabilities));

    if (
        bin_centers == NULL ||
        local_unlinkability == NULL ||
        mated_probabilities == NULL ||
        non_mated_probabilities == NULL
    ) {
        free(bin_centers);
        free(local_unlinkability);
        free(mated_probabilities);
        free(non_mated_probabilities);

        return UNLINKABILITY_METRIC_ALLOCATION_FAILURE;
    }

    for (size_t index = 0U; index < mated_count; ++index) {
        const size_t bin =
            score_bin(mated_scores[index], bin_count);

        mated_probabilities[bin] += 1.0;
    }

    for (size_t index = 0U; index < non_mated_count; ++index) {
        const size_t bin =
            score_bin(non_mated_scores[index], bin_count);

        non_mated_probabilities[bin] += 1.0;
    }

    for (size_t bin = 0U; bin < bin_count; ++bin) {
        mated_probabilities[bin] /= (double)mated_count;
        non_mated_probabilities[bin] /= (double)non_mated_count;

        bin_centers[bin] =
            ((double)bin + 0.5) /
            (double)bin_count;
    }

    double global_unlinkability = 0.0;

    for (size_t bin = 0U; bin < bin_count; ++bin) {
        const double mated_probability =
            mated_probabilities[bin];

        const double non_mated_probability =
            non_mated_probabilities[bin];

        double local = 0.0;

        if (
            mated_probability > 0.0 &&
            non_mated_probability == 0.0
        ) {
            /*
             * The likelihood ratio tends to infinity:
             * an observed score in this bin is only supported by
             * the mated empirical distribution.
             */
            local = 1.0;
        } else if (non_mated_probability > 0.0) {
            const double likelihood_ratio =
                mated_probability /
                non_mated_probability;

            const double weighted_ratio =
                omega * likelihood_ratio;

            if (weighted_ratio > 1.0) {
                local =
                    (weighted_ratio - 1.0) /
                    (weighted_ratio + 1.0);
            }
        }

        local_unlinkability[bin] = local;

        global_unlinkability +=
            mated_probability * local;
    }

    UnlinkabilityMetricResult generated = {
        .bin_count = bin_count,
        .omega = omega,
        .global_unlinkability = global_unlinkability,
        .bin_centers = bin_centers,
        .local_unlinkability = local_unlinkability,
        .mated_probabilities = mated_probabilities,
        .non_mated_probabilities = non_mated_probabilities
    };

    *result = generated;

    return UNLINKABILITY_METRIC_OK;
}
