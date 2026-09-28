#ifndef ERROR_RATE_SWEEP_H
#define ERROR_RATE_SWEEP_H

#include <stddef.h>

#include "evaluation.h"

typedef enum {
    ERROR_RATE_SWEEP_OK = 0,
    ERROR_RATE_SWEEP_INVALID_ARGUMENT,
    ERROR_RATE_SWEEP_INVALID_SCORE,
    ERROR_RATE_SWEEP_ALLOCATION_FAILURE,
    ERROR_RATE_SWEEP_EVALUATION_FAILURE
} ErrorRateSweepStatus;

typedef struct {
    size_t point_count;
    ErrorRates *points;
} ErrorRateSweepResult;

/*
 * Release memory owned by an error-rate sweep result.
 *
 * Safe for NULL and for a zero-initialized result.
 */
void error_rate_sweep_result_free(
    ErrorRateSweepResult *result
);

/*
 * Evaluate FMR and FNMR across every normalized Hamming-distance
 * operating point that can occur for a binary template of the
 * supplied hash length.
 *
 * Thresholds are:
 *
 *     0 / hash_length,
 *     1 / hash_length,
 *     ...
 *     hash_length / hash_length
 *
 * Therefore point_count is:
 *
 *     hash_length + 1
 *
 * Distances follow the convention used by the matcher and evaluation
 * modules:
 *
 *     lower distance = more similar
 *     distance <= threshold => match
 *
 * genuine_scores:
 *     Same-identity normalized Hamming distances.
 *
 * impostor_scores:
 *     Different-identity normalized Hamming distances.
 *
 * hash_length:
 *     Number of bits in the BioHash templates from which the scores
 *     were generated. Must be greater than zero.
 *
 * result:
 *     Receives an allocated array of ErrorRates entries.
 *
 * Release the result with error_rate_sweep_result_free().
 */
ErrorRateSweepStatus evaluate_error_rate_sweep(
    const double *genuine_scores,
    size_t genuine_count,
    const double *impostor_scores,
    size_t impostor_count,
    size_t hash_length,
    ErrorRateSweepResult *result
);

#endif
