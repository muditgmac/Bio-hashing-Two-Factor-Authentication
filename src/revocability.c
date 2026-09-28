#include "revocability.h"

#include <stdint.h>
#include <stdlib.h>

#include "matcher.h"

static int multiplication_would_overflow(
    size_t first,
    size_t second
)
{
    return second != 0U && first > SIZE_MAX / second;
}

RevocabilityStatus evaluate_revocability(
    const double *features,
    size_t feature_count,
    size_t hash_length,
    const BioHashConfig *token_configs,
    size_t token_count,
    RevocabilityResult *result
)
{
    if (
        features == NULL ||
        token_configs == NULL ||
        result == NULL ||
        feature_count == 0U ||
        hash_length == 0U ||
        hash_length > feature_count ||
        token_count < 2U
    ) {
        return REVOCABILITY_INVALID_ARGUMENT;
    }

    if (multiplication_would_overflow(token_count, hash_length)) {
        return REVOCABILITY_INVALID_ARGUMENT;
    }

    const size_t template_elements =
        token_count * hash_length;

    uint8_t *templates =
        malloc(template_elements * sizeof(*templates));

    if (templates == NULL) {
        return REVOCABILITY_ALLOCATION_FAILURE;
    }

    /*
     * Generate one protected template from the same biometric
     * feature vector under each independent token configuration.
     */
    for (size_t token = 0U; token < token_count; ++token) {
        uint8_t *current_template =
            &templates[token * hash_length];

        const BioHashStatus biohash_status =
            biohash_generate(
                features,
                feature_count,
                hash_length,
                &token_configs[token],
                current_template
            );

        if (biohash_status != BIOHASH_OK) {
            free(templates);
            return REVOCABILITY_BIOHASH_FAILURE;
        }
    }

    size_t comparison_count = 0U;
    size_t identical_template_pairs = 0U;

    double distance_sum = 0.0;
    double minimum_distance = 1.0;
    double maximum_distance = 0.0;

    /*
     * Compare every unique pair of token-derived templates:
     *
     *     (token 0, token 1)
     *     (token 0, token 2)
     *     ...
     *
     * This produces C(token_count, 2) cross-token comparisons.
     */
    for (size_t first = 0U; first < token_count; ++first) {
        const uint8_t *first_template =
            &templates[first * hash_length];

        for (
            size_t second = first + 1U;
            second < token_count;
            ++second
        ) {
            const uint8_t *second_template =
                &templates[second * hash_length];

            double distance = 0.0;

            const MatcherStatus matcher_status =
                normalized_hamming_distance(
                    first_template,
                    second_template,
                    hash_length,
                    &distance
                );

            if (matcher_status != MATCHER_OK) {
                free(templates);
                return REVOCABILITY_MATCHER_FAILURE;
            }

            if (distance < minimum_distance) {
                minimum_distance = distance;
            }

            if (distance > maximum_distance) {
                maximum_distance = distance;
            }

            if (distance == 0.0) {
                ++identical_template_pairs;
            }

            distance_sum += distance;
            ++comparison_count;
        }
    }

    RevocabilityResult revocability_result = {
        .token_count = token_count,
        .pairwise_comparisons = comparison_count,
        .identical_template_pairs = identical_template_pairs,
        .minimum_cross_token_distance = minimum_distance,
        .mean_cross_token_distance =
            distance_sum / (double)comparison_count,
        .maximum_cross_token_distance = maximum_distance
    };

    *result = revocability_result;

    free(templates);

    return REVOCABILITY_OK;
}
