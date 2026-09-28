#include "preprocessing.h"

#include <math.h>
#include <stdint.h>
#include <stdlib.h>

static int valid_matrix_shape(
    size_t sample_count,
    size_t feature_count
)
{
    if (
        sample_count == 0U ||
        feature_count == 0U
    ) {
        return 0;
    }

    if (
        sample_count >
        SIZE_MAX / feature_count
    ) {
        return 0;
    }

    return 1;
}

PreprocessingStatus sample_center_features(
    const double *input,
    size_t sample_count,
    size_t feature_count,
    double *output
)
{
    if (
        input == NULL ||
        output == NULL ||
        !valid_matrix_shape(
            sample_count,
            feature_count
        )
    ) {
        return PREPROCESSING_INVALID_ARGUMENT;
    }

    for (
        size_t sample = 0U;
        sample < sample_count;
        ++sample
    ) {
        const size_t offset =
            sample * feature_count;

        long double sum = 0.0L;

        for (
            size_t feature = 0U;
            feature < feature_count;
            ++feature
        ) {
            const double value =
                input[offset + feature];

            if (!isfinite(value)) {
                return PREPROCESSING_INVALID_DATA;
            }

            sum += (long double)value;
        }

        const long double mean_long =
            sum / (long double)feature_count;

        const double mean =
            (double)mean_long;

        if (!isfinite(mean)) {
            return PREPROCESSING_INVALID_DATA;
        }

        for (
            size_t feature = 0U;
            feature < feature_count;
            ++feature
        ) {
            const double centered =
                input[offset + feature] - mean;

            if (!isfinite(centered)) {
                return PREPROCESSING_INVALID_DATA;
            }

            output[offset + feature] =
                centered;
        }
    }

    return PREPROCESSING_OK;
}

PreprocessingStatus fit_population_centering(
    const double *input,
    size_t sample_count,
    size_t feature_count,
    PopulationCenteringModel *model
)
{
    if (
        input == NULL ||
        model == NULL ||
        !valid_matrix_shape(
            sample_count,
            feature_count
        )
    ) {
        return PREPROCESSING_INVALID_ARGUMENT;
    }

    if (
        feature_count >
        SIZE_MAX / sizeof(double)
    ) {
        return PREPROCESSING_INVALID_ARGUMENT;
    }

    double *mean =
        malloc(
            feature_count *
            sizeof(*mean)
        );

    if (mean == NULL) {
        return PREPROCESSING_ALLOCATION_FAILURE;
    }

    for (
        size_t feature = 0U;
        feature < feature_count;
        ++feature
    ) {
        long double sum = 0.0L;

        for (
            size_t sample = 0U;
            sample < sample_count;
            ++sample
        ) {
            const double value =
                input[
                    sample * feature_count +
                    feature
                ];

            if (!isfinite(value)) {
                free(mean);
                return PREPROCESSING_INVALID_DATA;
            }

            sum += (long double)value;
        }

        const long double feature_mean_long =
            sum / (long double)sample_count;

        mean[feature] =
            (double)feature_mean_long;

        if (!isfinite(mean[feature])) {
            free(mean);
            return PREPROCESSING_INVALID_DATA;
        }
    }

    free(model->population_mean);

    model->population_mean = mean;
    model->feature_count = feature_count;

    return PREPROCESSING_OK;
}

PreprocessingStatus apply_population_centering(
    const PopulationCenteringModel *model,
    const double *input,
    size_t sample_count,
    size_t feature_count,
    double *output
)
{
    if (
        model == NULL ||
        model->population_mean == NULL ||
        input == NULL ||
        output == NULL ||
        model->feature_count != feature_count ||
        !valid_matrix_shape(
            sample_count,
            feature_count
        )
    ) {
        return PREPROCESSING_INVALID_ARGUMENT;
    }

    for (
        size_t sample = 0U;
        sample < sample_count;
        ++sample
    ) {
        const size_t offset =
            sample * feature_count;

        for (
            size_t feature = 0U;
            feature < feature_count;
            ++feature
        ) {
            const double value =
                input[offset + feature];

            if (!isfinite(value)) {
                return PREPROCESSING_INVALID_DATA;
            }

            const double centered =
                value -
                model->population_mean[feature];

            if (!isfinite(centered)) {
                return PREPROCESSING_INVALID_DATA;
            }

            output[offset + feature] =
                centered;
        }
    }

    return PREPROCESSING_OK;
}

void population_centering_model_free(
    PopulationCenteringModel *model
)
{
    if (model == NULL) {
        return;
    }

    free(model->population_mean);

    model->population_mean = NULL;
    model->feature_count = 0U;
}
