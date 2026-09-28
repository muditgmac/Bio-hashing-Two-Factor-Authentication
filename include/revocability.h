#ifndef REVOCABILITY_H
#define REVOCABILITY_H

#include <stddef.h>

#include "biohash.h"

typedef enum {
    REVOCABILITY_OK = 0,
    REVOCABILITY_INVALID_ARGUMENT,
    REVOCABILITY_ALLOCATION_FAILURE,
    REVOCABILITY_BIOHASH_FAILURE,
    REVOCABILITY_MATCHER_FAILURE
} RevocabilityStatus;

typedef struct {
    size_t token_count;
    size_t pairwise_comparisons;
    size_t identical_template_pairs;

    double minimum_cross_token_distance;
    double mean_cross_token_distance;
    double maximum_cross_token_distance;
} RevocabilityResult;

/*
 * Evaluate renewability / revocability of a BioHash template.
 *
 * The same biometric feature vector is transformed using multiple
 * independent token configurations. Every pair of resulting protected
 * templates is compared using normalized Hamming distance.
 *
 * A successful renewable-template mechanism should be capable of
 * generating different protected templates after token replacement.
 *
 * This experiment measures cross-token diversity only. It does not,
 * by itself, establish unlinkability; unlinkability requires comparing
 * appropriate mated and non-mated cross-token score distributions.
 *
 * features:
 *     Input biometric feature vector.
 *
 * feature_count:
 *     Number of feature values.
 *
 * hash_length:
 *     Number of bits in each generated BioHash.
 *
 * token_configs:
 *     Array containing token-specific BioHash configurations.
 *
 * token_count:
 *     Number of token configurations. Must be at least two.
 *
 * result:
 *     Receives pairwise cross-token distance statistics.
 */
RevocabilityStatus evaluate_revocability(
    const double *features,
    size_t feature_count,
    size_t hash_length,
    const BioHashConfig *token_configs,
    size_t token_count,
    RevocabilityResult *result
);

#endif
