#ifndef UNLINKABILITY_METRIC_H
#define UNLINKABILITY_METRIC_H

#include <stddef.h>

typedef enum {
    UNLINKABILITY_METRIC_OK = 0,
    UNLINKABILITY_METRIC_INVALID_ARGUMENT,
    UNLINKABILITY_METRIC_INVALID_SCORE,
    UNLINKABILITY_METRIC_ALLOCATION_FAILURE
} UnlinkabilityMetricStatus;

/*
 * Empirical local/global unlinkability result.
 *
 * Scores are assumed to be normalized Hamming distances in [0, 1].
 *
 * For each histogram bin:
 *
 *     LR(s) = p(s | H_m) / p(s | H_nm)
 *
 * and, with prior ratio omega:
 *
 *     D(s) = 0
 *            if omega * LR(s) <= 1
 *
 *     D(s) = (omega * LR(s) - 1) /
 *            (omega * LR(s) + 1)
 *            otherwise
 *
 * The global unlinkability measure is the expectation of D(s)
 * under the mated distribution:
 *
 *     D_sys = sum_b p_m(b) * D(b)
 *
 * D_sys lies in [0, 1]:
 *
 *     0 -> no evidence for linking mated protected templates
 *     1 -> maximum linkability under the evaluated score model
 */
typedef struct {
    size_t bin_count;
    double omega;

    double global_unlinkability;

    double *bin_centers;
    double *local_unlinkability;

    double *mated_probabilities;
    double *non_mated_probabilities;
} UnlinkabilityMetricResult;

/*
 * Release memory owned by a metric result.
 *
 * Safe for NULL and zero-initialized result structures.
 */
void unlinkability_metric_result_free(
    UnlinkabilityMetricResult *result
);

/*
 * Estimate local and global unlinkability from cross-token
 * mated and non-mated normalized Hamming-distance scores.
 *
 * bin_count:
 *     Number of equal-width bins spanning [0, 1].
 *
 * omega:
 *     Prior-probability ratio:
 *
 *         p(H_m) / p(H_nm)
 *
 *     omega = 1.0 corresponds to equal priors.
 */
UnlinkabilityMetricStatus evaluate_unlinkability_metric(
    const double *mated_scores,
    size_t mated_count,
    const double *non_mated_scores,
    size_t non_mated_count,
    size_t bin_count,
    double omega,
    UnlinkabilityMetricResult *result
);

#endif
