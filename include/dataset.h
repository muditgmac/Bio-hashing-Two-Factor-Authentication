#ifndef DATASET_H
#define DATASET_H

#include <stddef.h>

typedef enum {
    DATASET_OK = 0,
    DATASET_INVALID_ARGUMENT,
    DATASET_IO_ERROR,
    DATASET_FORMAT_ERROR,
    DATASET_NONFINITE_VALUE,
    DATASET_ALLOCATION_FAILURE,
    DATASET_SIZE_OVERFLOW
} DatasetStatus;

typedef struct {
    size_t sample_count;
    size_t feature_count;

    /*
     * subject_ids:
     *     Array of length sample_count.
     *
     * features:
     *     Row-major matrix with shape:
     *
     *         sample_count x feature_count
     */
    size_t *subject_ids;
    double *features;
} Dataset;

/*
 * Release all memory owned by a Dataset.
 *
 * Safe for NULL and for a zero-initialized Dataset.
 */
void dataset_free(
    Dataset *dataset
);

/*
 * Load a numeric biometric feature dataset from CSV.
 *
 * Expected format:
 *
 *     subject_id,f1,f2,...,fN
 *     1,0.12,0.45,...,0.81
 *     1,0.11,0.47,...,0.79
 *     2,-0.31,0.22,...,0.14
 *
 * The header is optional. If present, its first field must be one of:
 *
 *     subject_id
 *     subject
 *     id
 *
 * Requirements:
 *
 * - every data row must contain one subject ID;
 * - every row must contain the same number of features;
 * - subject IDs must be non-negative integers;
 * - feature values must be finite real numbers;
 * - at least one sample and one feature must be present.
 */
DatasetStatus dataset_load_csv(
    const char *path,
    Dataset *result
);

#endif
