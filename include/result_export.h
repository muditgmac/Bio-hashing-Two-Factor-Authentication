#ifndef RESULT_EXPORT_H
#define RESULT_EXPORT_H

#include <stddef.h>

#include "biohash.h"
#include "experiment.h"
#include "error_rate_sweep.h"

typedef enum {
    RESULT_EXPORT_OK = 0,
    RESULT_EXPORT_INVALID_ARGUMENT,
    RESULT_EXPORT_INVALID_DATA,
    RESULT_EXPORT_IO_ERROR
} ResultExportStatus;

/*
 * Metadata describing the verification experiment whose results
 * are being exported.
 */
typedef struct {
    const char *input_path;

    size_t feature_count;
    size_t hash_length;

    /*
     * Preprocessing applied to feature vectors before BioHash
     * generation.
     */
    ExperimentPreprocessingMode preprocessing_mode;

    BioHashConfig biohash_config;
} VerificationExportMetadata;

/*
 * Export one verification experiment to three CSV files:
 *
 * summary_path:
 *     One-row experiment summary and configuration metadata.
 *
 * genuine_scores_path:
 *     Raw genuine normalized Hamming distances.
 *
 * impostor_scores_path:
 *     Raw impostor normalized Hamming distances.
 *
 * The function does not create parent directories. The caller must
 * ensure that all required directories already exist.
 */
ResultExportStatus export_verification_result_csv(
    const VerificationExportMetadata *metadata,
    const VerificationExperimentResult *result,
    const char *summary_path,
    const char *genuine_scores_path,
    const char *impostor_scores_path
);


/*
 * Export an evaluated error-rate operating-point sweep to CSV.
 *
 * The output contains one row for every evaluated decision
 * threshold, with columns:
 *
 *     threshold,fmr,fnmr
 *
 * The caller must ensure that the parent directory already exists.
 */
ResultExportStatus export_error_rate_sweep_csv(
    const ErrorRateSweepResult *sweep,
    const char *error_rates_path
);

#endif
