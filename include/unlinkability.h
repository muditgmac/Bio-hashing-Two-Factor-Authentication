#ifndef UNLINKABILITY_H
#define UNLINKABILITY_H

#include <stddef.h>

#include "biohash.h"

typedef enum {
    UNLINKABILITY_OK = 0,
    UNLINKABILITY_INVALID_ARGUMENT,
    UNLINKABILITY_SIZE_OVERFLOW,
    UNLINKABILITY_ALLOCATION_FAILURE,
    UNLINKABILITY_BIOHASH_FAILURE,
    UNLINKABILITY_MATCHER_FAILURE,
    UNLINKABILITY_INSUFFICIENT_COMPARISONS
} UnlinkabilityStatus;

typedef struct {
    size_t token_count;
    size_t token_pair_count;

    size_t mated_comparisons;
    size_t non_mated_comparisons;

    double minimum_mated_distance;
    double mean_mated_distance;
    double maximum_mated_distance;

    double minimum_non_mated_distance;
    double mean_non_mated_distance;
    double maximum_non_mated_distance;

    /*
     * Cross-token score distributions.
     *
     * mated_scores:
     *     Comparisons between protected templates belonging to
     *     the same subject but generated under different tokens.
     *
     * non_mated_scores:
     *     Comparisons between protected templates belonging to
     *     different subjects and generated under different tokens.
     *
     * Memory is owned by this result structure after a successful
     * call to evaluate_unlinkability_scores().
     *
     * Release it with unlinkability_result_free().
     */
    double *mated_scores;
    double *non_mated_scores;
} UnlinkabilityResult;

/*
 * Release score-distribution memory owned by an unlinkability result.
 *
 * It is safe to call this function with NULL or with a zero-initialized
 * result structure.
 */
void unlinkability_result_free(
    UnlinkabilityResult *result
);

/*
 * Generate cross-token mated and non-mated BioHash comparison scores.
 *
 * feature_matrix:
 *     Row-major matrix with shape:
 *
 *         sample_count x feature_count
 *
 * subject_ids:
 *     Identity label for each sample.
 *
 * token_configs:
 *     Independent BioHash token configurations. At least two are
 *     required because only cross-token comparisons are evaluated.
 *
 * For every unique pair of token configurations, every protected
 * template produced under the first token is compared with every
 * protected template produced under the second token.
 *
 * If the two samples have the same subject ID, the distance is added
 * to the mated cross-token distribution. Otherwise it is added to the
 * non-mated cross-token distribution.
 *
 * Both distributions must contain at least one comparison.
 */
UnlinkabilityStatus evaluate_unlinkability_scores(
    const double *feature_matrix,
    const size_t *subject_ids,
    size_t sample_count,
    size_t feature_count,
    size_t hash_length,
    const BioHashConfig *token_configs,
    size_t token_count,
    UnlinkabilityResult *result
);

#endif
