#include "unlinkability_experiment.h"

#include <math.h>

void unlinkability_experiment_result_free(
    UnlinkabilityExperimentResult *result
)
{
    if (result == NULL) {
        return;
    }

    unlinkability_result_free(&result->scores);
    unlinkability_metric_result_free(&result->metric);

    *result = (UnlinkabilityExperimentResult){0};
}

UnlinkabilityExperimentStatus run_unlinkability_experiment(
    const double *feature_matrix,
    const size_t *subject_ids,
    size_t sample_count,
    size_t feature_count,
    size_t hash_length,
    const BioHashConfig *token_configs,
    size_t token_count,
    size_t bin_count,
    double omega,
    UnlinkabilityExperimentResult *result
)
{
    if (
        feature_matrix == NULL ||
        subject_ids == NULL ||
        token_configs == NULL ||
        result == NULL ||
        sample_count < 2U ||
        feature_count == 0U ||
        hash_length == 0U ||
        hash_length > feature_count ||
        token_count < 2U ||
        bin_count == 0U ||
        !isfinite(omega) ||
        omega <= 0.0
    ) {
        return UNLINKABILITY_EXPERIMENT_INVALID_ARGUMENT;
    }

    /*
     * Ensure that a failure occurring later cannot leave a partially
     * populated result visible to the caller.
     */
    *result = (UnlinkabilityExperimentResult){0};

    UnlinkabilityResult scores = {0};

    const UnlinkabilityStatus score_status =
        evaluate_unlinkability_scores(
            feature_matrix,
            subject_ids,
            sample_count,
            feature_count,
            hash_length,
            token_configs,
            token_count,
            &scores
        );

    if (score_status != UNLINKABILITY_OK) {
        unlinkability_result_free(&scores);

        return
            UNLINKABILITY_EXPERIMENT_SCORE_GENERATION_FAILURE;
    }

    UnlinkabilityMetricResult metric = {0};

    const UnlinkabilityMetricStatus metric_status =
        evaluate_unlinkability_metric(
            scores.mated_scores,
            scores.mated_comparisons,
            scores.non_mated_scores,
            scores.non_mated_comparisons,
            bin_count,
            omega,
            &metric
        );

    if (metric_status != UNLINKABILITY_METRIC_OK) {
        unlinkability_result_free(&scores);
        unlinkability_metric_result_free(&metric);

        return UNLINKABILITY_EXPERIMENT_METRIC_FAILURE;
    }

    UnlinkabilityExperimentResult generated = {
        .sample_count = sample_count,
        .feature_count = feature_count,
        .hash_length = hash_length,
        .token_count = token_count,
        .scores = scores,
        .metric = metric
    };

    *result = generated;

    return UNLINKABILITY_EXPERIMENT_OK;
}
