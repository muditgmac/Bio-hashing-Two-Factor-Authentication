#ifndef MATCHER_H
#define MATCHER_H

#include <stddef.h>
#include <stdint.h>

typedef enum {
    MATCHER_OK = 0,
    MATCHER_INVALID_ARGUMENT,
    MATCHER_INVALID_TEMPLATE
} MatcherStatus;

/*
 * Compute the Hamming distance between two binary BioHash templates.
 *
 * distance:
 *     Receives the number of bit positions at which the templates differ.
 */
MatcherStatus hamming_distance(
    const uint8_t *first,
    const uint8_t *second,
    size_t length,
    size_t *distance
);

/*
 * Compute normalized Hamming distance in [0, 1].
 *
 * normalized_distance =
 *     differing_bits / total_bits
 */
MatcherStatus normalized_hamming_distance(
    const uint8_t *first,
    const uint8_t *second,
    size_t length,
    double *distance
);

/*
 * Perform threshold-based BioHash verification.
 *
 * Two templates are considered a match when their normalized Hamming
 * distance is less than or equal to max_distance.
 */
MatcherStatus biohash_match(
    const uint8_t *first,
    const uint8_t *second,
    size_t length,
    double max_distance,
    int *is_match
);

#endif
