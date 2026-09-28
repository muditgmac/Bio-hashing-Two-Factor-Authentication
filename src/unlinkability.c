#include "unlinkability.h"

#include "matcher.h"

#include <stdint.h>
#include <stdlib.h>

static int checked_multiply(
    size_t first,
    size_t second,
    size_t *product
)
{
    if (product == NULL) {
        return 0;
    }

    if (
        first != 0U &&
        second > SIZE_MAX / first
    ) {
        return 0;
    }

    *product = first * second;

    return 1;
}

static int token_pair_count(
    size_t token_count,
    size_t *pair_count
)
{
    if (
        token_count < 2U ||
        pair_count == NULL
    ) {
        return 0;
    }

    /*
     * Compute:
     *
     *     token_count * (token_count - 1) / 2
     *
     * while dividing before multiplication to reduce the chance
     * of an intermediate overflow.
     */
    size_t first = token_count;
    size_t second = token_count - 1U;

    if ((first % 2U) == 0U) {
        first /= 2U;
    } else {
        second /= 2U;
    }

    return checked_multiply(
        first,
        second,
        pair_count
    );
}

static int count_sample_pair_types(
    const size_t *subject_ids,
    size_t sample_count,
    size_t *mated_count,
    size_t *non_mated_count
)
{
    if (
        subject_ids == NULL ||
        mated_count == NULL ||
        non_mated_count == NULL
    ) {
        return 0;
    }

    size_t mated = 0U;
    size_t non_mated = 0U;

    /*
     * These are cross-application/cross-token comparisons.
     *
     * Therefore sample A under token 1 is compared against every
     * sample B under token 2.
     */
    for (size_t first = 0U; first < sample_count; ++first) {
        for (size_t second = 0U; second < sample_count; ++second) {
            if (subject_ids[first] == subject_ids[second]) {
                if (mated == SIZE_MAX) {
                    return 0;
                }

                ++mated;
            } else {
                if (non_mated == SIZE_MAX) {
                    return 0;
                }

                ++non_mated;
            }
        }
    }

    *mated_count = mated;
    *non_mated_count = non_mated;

    return 1;
}

void unlinkability_result_free(
    UnlinkabilityResult *result
)
{
    if (result == NULL) {
        return;
    }

    free(result->mated_scores);
    free(result->non_mated_scores);

    *result = (UnlinkabilityResult){0};
}

UnlinkabilityStatus evaluate_unlinkability_scores(
    const double *feature_matrix,
    const size_t *subject_ids,
    size_t sample_count,
    size_t feature_count,
    size_t hash_length,
    const BioHashConfig *token_configs,
    size_t token_count,
    UnlinkabilityResult *result
)
{
    if (result != NULL) {
        *result = (UnlinkabilityResult){0};
    }

    if (
        feature_matrix == NULL ||
        subject_ids == NULL ||
        token_configs == NULL ||
        result == NULL ||
        sample_count == 0U ||
        feature_count == 0U ||
        hash_length == 0U ||
        hash_length > feature_count ||
        token_count < 2U
    ) {
        return UNLINKABILITY_INVALID_ARGUMENT;
    }

    size_t feature_elements = 0U;

    if (!checked_multiply(
            sample_count,
            feature_count,
            &feature_elements
        )) {
        return UNLINKABILITY_SIZE_OVERFLOW;
    }

    /*
     * feature_elements is used as an overflow check for later
     * row-major indexing.
     */
    (void)feature_elements;

    size_t pair_count = 0U;

    if (!token_pair_count(
            token_count,
            &pair_count
        )) {
        return UNLINKABILITY_SIZE_OVERFLOW;
    }

    size_t mated_per_token_pair = 0U;
    size_t non_mated_per_token_pair = 0U;

    if (!count_sample_pair_types(
            subject_ids,
            sample_count,
            &mated_per_token_pair,
            &non_mated_per_token_pair
        )) {
        return UNLINKABILITY_SIZE_OVERFLOW;
    }

    if (
        mated_per_token_pair == 0U ||
        non_mated_per_token_pair == 0U
    ) {
        return UNLINKABILITY_INSUFFICIENT_COMPARISONS;
    }

    size_t mated_count = 0U;
    size_t non_mated_count = 0U;

    if (
        !checked_multiply(
            mated_per_token_pair,
            pair_count,
            &mated_count
        ) ||
        !checked_multiply(
            non_mated_per_token_pair,
            pair_count,
            &non_mated_count
        )
    ) {
        return UNLINKABILITY_SIZE_OVERFLOW;
    }

    size_t template_count = 0U;

    if (!checked_multiply(
            token_count,
            sample_count,
            &template_count
        )) {
        return UNLINKABILITY_SIZE_OVERFLOW;
    }

    size_t template_elements = 0U;

    if (!checked_multiply(
            template_count,
            hash_length,
            &template_elements
        )) {
        return UNLINKABILITY_SIZE_OVERFLOW;
    }

    size_t mated_bytes = 0U;
    size_t non_mated_bytes = 0U;

    if (
        !checked_multiply(
            mated_count,
            sizeof(double),
            &mated_bytes
        ) ||
        !checked_multiply(
            non_mated_count,
            sizeof(double),
            &non_mated_bytes
        )
    ) {
        return UNLINKABILITY_SIZE_OVERFLOW;
    }

    uint8_t *templates =
        malloc(template_elements * sizeof(*templates));

    double *mated_scores =
        malloc(mated_bytes);

    double *non_mated_scores =
        malloc(non_mated_bytes);

    if (
        templates == NULL ||
        mated_scores == NULL ||
        non_mated_scores == NULL
    ) {
        free(templates);
        free(mated_scores);
        free(non_mated_scores);

        return UNLINKABILITY_ALLOCATION_FAILURE;
    }

    /*
     * Generate one protected BioHash template for every
     * sample/token combination.
     */
    for (size_t token = 0U; token < token_count; ++token) {
        for (size_t sample = 0U; sample < sample_count; ++sample) {
            const double *features =
                &feature_matrix[sample * feature_count];

            uint8_t *output =
                &templates[
                    ((token * sample_count) + sample) *
                    hash_length
                ];

            const BioHashStatus status =
                biohash_generate(
                    features,
                    feature_count,
                    hash_length,
                    &token_configs[token],
                    output
                );

            if (status != BIOHASH_OK) {
                free(templates);
                free(mated_scores);
                free(non_mated_scores);

                return UNLINKABILITY_BIOHASH_FAILURE;
            }
        }
    }

    size_t mated_index = 0U;
    size_t non_mated_index = 0U;

    double mated_sum = 0.0;
    double non_mated_sum = 0.0;

    double minimum_mated = 1.0;
    double maximum_mated = 0.0;

    double minimum_non_mated = 1.0;
    double maximum_non_mated = 0.0;

    /*
     * Only compare templates generated under different tokens.
     *
     * Each unordered token pair is evaluated exactly once.
     */
    for (size_t first_token = 0U;
         first_token < token_count;
         ++first_token) {

        for (size_t second_token = first_token + 1U;
             second_token < token_count;
             ++second_token) {

            for (size_t first_sample = 0U;
                 first_sample < sample_count;
                 ++first_sample) {

                const uint8_t *first_template =
                    &templates[
                        (
                            (first_token * sample_count) +
                            first_sample
                        ) * hash_length
                    ];

                for (size_t second_sample = 0U;
                     second_sample < sample_count;
                     ++second_sample) {

                    const uint8_t *second_template =
                        &templates[
                            (
                                (second_token * sample_count) +
                                second_sample
                            ) * hash_length
                        ];

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
                        free(mated_scores);
                        free(non_mated_scores);

                        return UNLINKABILITY_MATCHER_FAILURE;
                    }

                    if (
                        subject_ids[first_sample] ==
                        subject_ids[second_sample]
                    ) {
                        mated_scores[mated_index] = distance;
                        ++mated_index;

                        mated_sum += distance;

                        if (distance < minimum_mated) {
                            minimum_mated = distance;
                        }

                        if (distance > maximum_mated) {
                            maximum_mated = distance;
                        }
                    } else {
                        non_mated_scores[non_mated_index] =
                            distance;

                        ++non_mated_index;

                        non_mated_sum += distance;

                        if (distance < minimum_non_mated) {
                            minimum_non_mated = distance;
                        }

                        if (distance > maximum_non_mated) {
                            maximum_non_mated = distance;
                        }
                    }
                }
            }
        }
    }

    /*
     * These checks also protect against implementation mistakes in
     * the comparison loops drifting away from the pre-computed sizes.
     */
    if (
        mated_index != mated_count ||
        non_mated_index != non_mated_count
    ) {
        free(templates);
        free(mated_scores);
        free(non_mated_scores);

        return UNLINKABILITY_INSUFFICIENT_COMPARISONS;
    }

    UnlinkabilityResult generated_result = {
        .token_count = token_count,
        .token_pair_count = pair_count,

        .mated_comparisons = mated_count,
        .non_mated_comparisons = non_mated_count,

        .minimum_mated_distance = minimum_mated,
        .mean_mated_distance =
            mated_sum / (double)mated_count,
        .maximum_mated_distance = maximum_mated,

        .minimum_non_mated_distance = minimum_non_mated,
        .mean_non_mated_distance =
            non_mated_sum / (double)non_mated_count,
        .maximum_non_mated_distance = maximum_non_mated,

        .mated_scores = mated_scores,
        .non_mated_scores = non_mated_scores
    };

    *result = generated_result;

    free(templates);

    return UNLINKABILITY_OK;
}
