#include "experiment.h"

#include <stdint.h>
#include <stdlib.h>

#include "matcher.h"

static int checked_product(
    size_t first,
    size_t second,
    size_t *result
)
{
    if (result == NULL) {
        return 0;
    }

    if (
        first != 0U &&
        second > SIZE_MAX / first
    ) {
        return 0;
    }

    *result = first * second;

    return 1;
}

static ExperimentStatus count_comparison_types(
    const size_t *subject_ids,
    size_t sample_count,
    size_t *genuine_count,
    size_t *impostor_count
)
{
    if (
        subject_ids == NULL ||
        genuine_count == NULL ||
        impostor_count == NULL
    ) {
        return EXPERIMENT_INVALID_ARGUMENT;
    }

    *genuine_count = 0U;
    *impostor_count = 0U;

    for (size_t first = 0U; first < sample_count; ++first) {
        for (
            size_t second = first + 1U;
            second < sample_count;
            ++second
        ) {
            if (subject_ids[first] == subject_ids[second]) {
                if (*genuine_count == SIZE_MAX) {
                    return EXPERIMENT_INVALID_ARGUMENT;
                }

                ++(*genuine_count);
            } else {
                if (*impostor_count == SIZE_MAX) {
                    return EXPERIMENT_INVALID_ARGUMENT;
                }

                ++(*impostor_count);
            }
        }
    }

    if (
        *genuine_count == 0U ||
        *impostor_count == 0U
    ) {
        return EXPERIMENT_INSUFFICIENT_COMPARISONS;
    }

    return EXPERIMENT_OK;
}

ExperimentStatus run_verification_experiment(
    const double *feature_matrix,
    const size_t *subject_ids,
    size_t sample_count,
    size_t feature_count,
    size_t hash_length,
    const BioHashConfig *config,
    VerificationExperimentResult *result
)
{
    if (
        feature_matrix == NULL ||
        subject_ids == NULL ||
        config == NULL ||
        result == NULL ||
        sample_count < 2U ||
        feature_count == 0U ||
        hash_length == 0U ||
        hash_length > feature_count
    ) {
        return EXPERIMENT_INVALID_ARGUMENT;
    }

    size_t feature_element_count = 0U;
    size_t template_element_count = 0U;

    if (
        !checked_product(
            sample_count,
            feature_count,
            &feature_element_count
        ) ||
        !checked_product(
            sample_count,
            hash_length,
            &template_element_count
        )
    ) {
        return EXPERIMENT_INVALID_ARGUMENT;
    }

    /*
     * feature_element_count is intentionally calculated as part of
     * overflow validation even though indexing below uses sample
     * offsets rather than this total directly.
     */
    (void)feature_element_count;

    size_t genuine_count = 0U;
    size_t impostor_count = 0U;

    const ExperimentStatus count_status =
        count_comparison_types(
            subject_ids,
            sample_count,
            &genuine_count,
            &impostor_count
        );

    if (count_status != EXPERIMENT_OK) {
        return count_status;
    }

    uint8_t *templates =
        malloc(template_element_count * sizeof(*templates));

    double *genuine_scores =
        malloc(genuine_count * sizeof(*genuine_scores));

    double *impostor_scores =
        malloc(impostor_count * sizeof(*impostor_scores));

    if (
        templates == NULL ||
        genuine_scores == NULL ||
        impostor_scores == NULL
    ) {
        free(templates);
        free(genuine_scores);
        free(impostor_scores);

        return EXPERIMENT_ALLOCATION_FAILURE;
    }

    for (size_t sample = 0U; sample < sample_count; ++sample) {
        const double *features =
            &feature_matrix[sample * feature_count];

        uint8_t *biohash =
            &templates[sample * hash_length];

        const BioHashStatus biohash_status =
            biohash_generate(
                features,
                feature_count,
                hash_length,
                config,
                biohash
            );

        if (biohash_status != BIOHASH_OK) {
            free(templates);
            free(genuine_scores);
            free(impostor_scores);

            return EXPERIMENT_BIOHASH_FAILURE;
        }
    }

    size_t genuine_index = 0U;
    size_t impostor_index = 0U;

    double genuine_sum = 0.0;
    double impostor_sum = 0.0;

    for (size_t first = 0U; first < sample_count; ++first) {
        const uint8_t *first_template =
            &templates[first * hash_length];

        for (
            size_t second = first + 1U;
            second < sample_count;
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
                free(genuine_scores);
                free(impostor_scores);

                return EXPERIMENT_MATCHER_FAILURE;
            }

            if (subject_ids[first] == subject_ids[second]) {
                genuine_scores[genuine_index] = distance;
                genuine_sum += distance;
                ++genuine_index;
            } else {
                impostor_scores[impostor_index] = distance;
                impostor_sum += distance;
                ++impostor_index;
            }
        }
    }

    EqualErrorRate equal_error_rate;

    const EvaluationStatus evaluation_status =
        estimate_equal_error_rate(
            genuine_scores,
            genuine_count,
            impostor_scores,
            impostor_count,
            &equal_error_rate
        );

    if (evaluation_status != EVALUATION_OK) {
        free(templates);
        free(genuine_scores);
        free(impostor_scores);

        return EXPERIMENT_EVALUATION_FAILURE;
    }

    VerificationExperimentResult experiment_result = {
        .sample_count = sample_count,
        .genuine_comparisons = genuine_count,
        .impostor_comparisons = impostor_count,
        .mean_genuine_distance =
            genuine_sum / (double)genuine_count,
        .mean_impostor_distance =
            impostor_sum / (double)impostor_count,
        .equal_error_rate = equal_error_rate
    };

    *result = experiment_result;

    free(templates);
    free(genuine_scores);
    free(impostor_scores);

    return EXPERIMENT_OK;
}
