#ifndef EVALUATION_H
#define EVALUATION_H

#include <stddef.h>

typedef enum {
    EVALUATION_OK = 0,
    EVALUATION_INVALID_ARGUMENT,
    EVALUATION_INVALID_SCORE
} EvaluationStatus;

typedef struct {
    double threshold;
    double fmr;
    double fnmr;
} ErrorRates;

typedef struct {
    double threshold;
    double eer;
    double fmr;
    double fnmr;
} EqualErrorRate;

/*
 * Evaluate comparison-level biometric error rates.
 *
 * Distances use the convention already used by the matcher:
 *
 *     lower distance = more similar
 *     distance <= threshold => match
 *
 * Genuine scores:
 *     Comparisons belonging to the same identity.
 *
 * Impostor scores:
 *     Comparisons belonging to different identities.
 */
EvaluationStatus evaluate_error_rates(
    const double *genuine_scores,
    size_t genuine_count,
    const double *impostor_scores,
    size_t impostor_count,
    double threshold,
    ErrorRates *result
);

/*
 * Estimate the equal-error operating point by evaluating every
 * distinct observed score as a candidate decision threshold.
 *
 * The returned EER is the mean of FMR and FNMR at the candidate
 * threshold having the smallest absolute difference between them.
 */
EvaluationStatus estimate_equal_error_rate(
    const double *genuine_scores,
    size_t genuine_count,
    const double *impostor_scores,
    size_t impostor_count,
    EqualErrorRate *result
);

#endif
