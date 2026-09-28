#ifndef BIOHASH_H
#define BIOHASH_H

#include <stddef.h>
#include <stdint.h>

typedef enum {
    BIOHASH_OK = 0,
    BIOHASH_INVALID_ARGUMENT,
    BIOHASH_ALLOCATION_FAILURE,
    BIOHASH_BBS_FAILURE,
    BIOHASH_DCT_FAILURE,
    BIOHASH_PROJECTION_FAILURE
} BioHashStatus;

typedef struct {
    uint64_t p;
    uint64_t q;
    uint64_t seed;
    double threshold;
    double orthogonality_tolerance;
} BioHashConfig;

/*
 * Generate a token-dependent binary BioHash.
 *
 * features:
 *     Input biometric/feature vector.
 *
 * feature_count:
 *     Number of elements in the feature vector.
 *
 * hash_length:
 *     Number of output BioHash bits. Must not exceed feature_count.
 *
 * config:
 *     BBS parameters and quantization settings.
 *
 * output:
 *     Receives hash_length binary values (0 or 1).
 */
BioHashStatus biohash_generate(
    const double *features,
    size_t feature_count,
    size_t hash_length,
    const BioHashConfig *config,
    uint8_t *output
);

#endif
