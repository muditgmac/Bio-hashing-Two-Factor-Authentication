#ifndef UNLINKABILITY_EXPERIMENT_H
#define UNLINKABILITY_EXPERIMENT_H

#include <stddef.h>

#include "biohash.h"
#include "unlinkability.h"
#include "unlinkability_metric.h"

typedef enum {
    UNLINKABILITY_EXPERIMENT_OK = 0,
    UNLINKABILITY_EXPERIMENT_INVALID_ARGUMENT,
    UNLINKABILITY_EXPERIMENT_SCORE_GENERATION_FAILURE,
    UNLINKABILITY_EXPERIMENT_METRIC_FAILURE
} UnlinkabilityExperimentStatus;

/*
 * Complete result of an end-to-end unlinkability experiment.
 *
 * The score-generation stage contains the cross-token mated and
 * non-mated Hamming-distance distributions together with descriptive
 * statistics.
 *
 * The metric stage contains the empirical score distributions,
 * score-wise local unlinkability values, and the global D_sys value.
 *
 * Dynamic memory inside both nested result structures is owned by
 * this result after a successful experiment. Release it with
 * unlinkability_experiment_result_free().
 */
typedef struct {
    size_t sample_count;
    size_t feature_count;
    size_t hash_length;
    size_t token_count;

    UnlinkabilityResult scores;
    UnlinkabilityMetricResult metric;
} UnlinkabilityExperimentResult;

/*
 * Release all dynamically allocated memory owned by a successful
 * unlinkability experiment result.
 *
 * Safe for NULL and zero-initialized result structures.
 */
void unlinkability_experiment_result_free(
    UnlinkabilityExperimentResult *result
);

/*
 * Run a complete BioHash unlinkability experiment.
 *
 * Stage 1:
 *     Generate protected BioHash templates for every sample under
 *     every token configuration and construct cross-token mated and
 *     non-mated normalized Hamming-distance distributions.
 *
 * Stage 2:
 *     Estimate local unlinkability D(s) and the global system-level
 *     unlinkability measure D_sys from those score distributions.
 *
 * feature_matrix:
 *     Row-major matrix with shape:
 *
 *         sample_count x feature_count
 *
 * subject_ids:
 *     Identity label associated with each sample.
 *
 * token_configs:
 *     Independent BioHash token configurations. At least two are
 *     required.
 *
 * bin_count:
 *     Number of equal-width bins used by the quantitative
 *     unlinkability estimator over normalized scores in [0, 1].
 *
 * omega:
 *     Prior-probability ratio used by the unlinkability metric.
 *     omega = 1.0 represents equal mated/non-mated priors.
 *
 * result:
 *     Receives both the score distributions and the quantitative
 *     unlinkability metric.
 */
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
);

#endif
