#ifndef EXPERIMENT_H
#define EXPERIMENT_H

#include <stddef.h>

#include "biohash.h"
#include "evaluation.h"

typedef enum {
    EXPERIMENT_OK = 0,
    EXPERIMENT_INVALID_ARGUMENT,
    EXPERIMENT_ALLOCATION_FAILURE,
    EXPERIMENT_BIOHASH_FAILURE,
    EXPERIMENT_MATCHER_FAILURE,
    EXPERIMENT_EVALUATION_FAILURE,
    EXPERIMENT_INSUFFICIENT_COMPARISONS
} ExperimentStatus;

typedef struct {
    size_t sample_count;
    size_t genuine_comparisons;
    size_t impostor_comparisons;

    double mean_genuine_distance;
    double mean_impostor_distance;

    EqualErrorRate equal_error_rate;
} VerificationExperimentResult;

/*
 * Run a deterministic one-to-one BioHash verification experiment.
 *
 * feature_matrix:
 *     Row-major matrix with shape:
 *
 *         sample_count x feature_count
 *
 * subject_ids:
 *     Identity label associated with every sample.
 *
 * Samples sharing the same subject ID produce genuine comparisons.
 * Samples belonging to different subject IDs produce impostor comparisons.
 *
 * A BioHash is generated for every sample using the same BioHashConfig,
 * after which all unique sample pairs are compared using normalized
 * Hamming distance.
 *
 * The resulting genuine and impostor score distributions are then used
 * to estimate the equal-error operating point.
 */
ExperimentStatus run_verification_experiment(
    const double *feature_matrix,
    const size_t *subject_ids,
    size_t sample_count,
    size_t feature_count,
    size_t hash_length,
    const BioHashConfig *config,
    VerificationExperimentResult *result
);

#endif
