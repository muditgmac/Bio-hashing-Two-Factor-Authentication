#ifndef PREPROCESSING_H
#define PREPROCESSING_H

#include <stddef.h>

typedef enum {
    PREPROCESSING_OK = 0,
    PREPROCESSING_INVALID_ARGUMENT,
    PREPROCESSING_INVALID_DATA,
    PREPROCESSING_ALLOCATION_FAILURE
} PreprocessingStatus;

typedef struct {
    size_t feature_count;
    double *population_mean;
} PopulationCenteringModel;

/*
 * Subtract the mean of each individual sample from all of that sample's
 * features.
 *
 * input and output may point to the same array.
 */
PreprocessingStatus sample_center_features(
    const double *input,
    size_t sample_count,
    size_t feature_count,
    double *output
);

/*
 * Fit one feature-wise population mean using the supplied development
 * samples.
 *
 * The model must be zero-initialized before first use:
 *
 *     PopulationCenteringModel model = {0};
 *
 * A successful refit replaces the previous fitted mean.
 */
PreprocessingStatus fit_population_centering(
    const double *input,
    size_t sample_count,
    size_t feature_count,
    PopulationCenteringModel *model
);

/*
 * Apply a previously fitted feature-wise population mean.
 *
 * input and output may point to the same array.
 */
PreprocessingStatus apply_population_centering(
    const PopulationCenteringModel *model,
    const double *input,
    size_t sample_count,
    size_t feature_count,
    double *output
);

void population_centering_model_free(
    PopulationCenteringModel *model
);

#endif
