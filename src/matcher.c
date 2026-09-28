#include "matcher.h"

#include <math.h>
#include <stddef.h>
#include <stdint.h>

static int is_valid_binary_template(
    const uint8_t *values,
    size_t length
)
{
    if (values == NULL || length == 0U) {
        return 0;
    }

    for (size_t index = 0U; index < length; ++index) {
        if (values[index] != 0U && values[index] != 1U) {
            return 0;
        }
    }

    return 1;
}

MatcherStatus hamming_distance(
    const uint8_t *first,
    const uint8_t *second,
    size_t length,
    size_t *distance
)
{
    if (
        first == NULL ||
        second == NULL ||
        distance == NULL ||
        length == 0U
    ) {
        return MATCHER_INVALID_ARGUMENT;
    }

    if (
        !is_valid_binary_template(first, length) ||
        !is_valid_binary_template(second, length)
    ) {
        return MATCHER_INVALID_TEMPLATE;
    }

    size_t differences = 0U;

    for (size_t index = 0U; index < length; ++index) {
        if (first[index] != second[index]) {
            ++differences;
        }
    }

    *distance = differences;

    return MATCHER_OK;
}

MatcherStatus normalized_hamming_distance(
    const uint8_t *first,
    const uint8_t *second,
    size_t length,
    double *distance
)
{
    if (distance == NULL) {
        return MATCHER_INVALID_ARGUMENT;
    }

    size_t raw_distance = 0U;

    const MatcherStatus status =
        hamming_distance(
            first,
            second,
            length,
            &raw_distance
        );

    if (status != MATCHER_OK) {
        return status;
    }

    *distance =
        (double)raw_distance /
        (double)length;

    return MATCHER_OK;
}

MatcherStatus biohash_match(
    const uint8_t *first,
    const uint8_t *second,
    size_t length,
    double max_distance,
    int *is_match
)
{
    if (
        is_match == NULL ||
        !isfinite(max_distance) ||
        max_distance < 0.0 ||
        max_distance > 1.0
    ) {
        return MATCHER_INVALID_ARGUMENT;
    }

    double distance = 0.0;

    const MatcherStatus status =
        normalized_hamming_distance(
            first,
            second,
            length,
            &distance
        );

    if (status != MATCHER_OK) {
        return status;
    }

    *is_match = distance <= max_distance ? 1 : 0;

    return MATCHER_OK;
}
