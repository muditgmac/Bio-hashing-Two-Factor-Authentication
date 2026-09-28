#include "result_export.h"

#include <inttypes.h>
#include <math.h>
#include <stdio.h>

static int path_is_valid(
    const char *path
)
{
    return
        path != NULL &&
        path[0] != '\0';
}

static int normalized_value_is_valid(
    double value
)
{
    return
        isfinite(value) &&
        value >= 0.0 &&
        value <= 1.0;
}

static int metadata_is_valid(
    const VerificationExportMetadata *metadata
)
{
    if (
        metadata == NULL ||
        metadata->input_path == NULL ||
        metadata->input_path[0] == '\0' ||
        metadata->feature_count == 0U ||
        metadata->hash_length == 0U ||
        metadata->hash_length > metadata->feature_count
    ) {
        return 0;
    }

    if (
        !isfinite(metadata->biohash_config.threshold) ||
        !isfinite(
            metadata->biohash_config.orthogonality_tolerance
        ) ||
        metadata->biohash_config.orthogonality_tolerance < 0.0
    ) {
        return 0;
    }

    return 1;
}

static int result_is_valid(
    const VerificationExperimentResult *result
)
{
    if (
        result == NULL ||
        result->sample_count == 0U ||
        result->genuine_comparisons == 0U ||
        result->impostor_comparisons == 0U ||
        result->genuine_scores == NULL ||
        result->impostor_scores == NULL
    ) {
        return 0;
    }

    if (
        !normalized_value_is_valid(
            result->mean_genuine_distance
        ) ||
        !normalized_value_is_valid(
            result->mean_impostor_distance
        ) ||
        !normalized_value_is_valid(
            result->equal_error_rate.threshold
        ) ||
        !normalized_value_is_valid(
            result->equal_error_rate.fmr
        ) ||
        !normalized_value_is_valid(
            result->equal_error_rate.fnmr
        ) ||
        !normalized_value_is_valid(
            result->equal_error_rate.eer
        )
    ) {
        return 0;
    }

    for (
        size_t index = 0U;
        index < result->genuine_comparisons;
        ++index
    ) {
        if (
            !normalized_value_is_valid(
                result->genuine_scores[index]
            )
        ) {
            return 0;
        }
    }

    for (
        size_t index = 0U;
        index < result->impostor_comparisons;
        ++index
    ) {
        if (
            !normalized_value_is_valid(
                result->impostor_scores[index]
            )
        ) {
            return 0;
        }
    }

    return 1;
}

/*
 * Write a standards-compatible quoted CSV text field.
 *
 * Embedded quote characters are escaped by doubling them.
 */
static int write_csv_string(
    FILE *stream,
    const char *value
)
{
    if (fputc('"', stream) == EOF) {
        return 0;
    }

    for (
        const char *character = value;
        *character != '\0';
        ++character
    ) {
        if (*character == '"') {
            if (fputc('"', stream) == EOF) {
                return 0;
            }
        }

        if (fputc(*character, stream) == EOF) {
            return 0;
        }
    }

    return fputc('"', stream) != EOF;
}

static ResultExportStatus write_summary_csv(
    const char *path,
    const VerificationExportMetadata *metadata,
    const VerificationExperimentResult *result
)
{
    FILE *stream = fopen(path, "w");

    if (stream == NULL) {
        return RESULT_EXPORT_IO_ERROR;
    }

    if (
        fputs(
            "input_file,"
            "sample_count,"
            "feature_count,"
            "hash_length,"
            "bbs_p,"
            "bbs_q,"
            "bbs_seed,"
            "quantization_threshold,"
            "orthogonality_tolerance,"
            "genuine_comparisons,"
            "impostor_comparisons,"
            "mean_genuine_distance,"
            "mean_impostor_distance,"
            "eer_threshold,"
            "fmr,"
            "fnmr,"
            "eer\n",
            stream
        ) == EOF
    ) {
        (void)fclose(stream);
        return RESULT_EXPORT_IO_ERROR;
    }

    if (!write_csv_string(stream, metadata->input_path)) {
        (void)fclose(stream);
        return RESULT_EXPORT_IO_ERROR;
    }

    if (
        fprintf(
            stream,
            ",%zu,%zu,%zu,"
            "%" PRIu64 ","
            "%" PRIu64 ","
            "%" PRIu64 ","
            "%.17g,%.17g,"
            "%zu,%zu,"
            "%.17g,%.17g,"
            "%.17g,%.17g,%.17g,%.17g\n",
            result->sample_count,
            metadata->feature_count,
            metadata->hash_length,
            metadata->biohash_config.p,
            metadata->biohash_config.q,
            metadata->biohash_config.seed,
            metadata->biohash_config.threshold,
            metadata->biohash_config.orthogonality_tolerance,
            result->genuine_comparisons,
            result->impostor_comparisons,
            result->mean_genuine_distance,
            result->mean_impostor_distance,
            result->equal_error_rate.threshold,
            result->equal_error_rate.fmr,
            result->equal_error_rate.fnmr,
            result->equal_error_rate.eer
        ) < 0
    ) {
        (void)fclose(stream);
        return RESULT_EXPORT_IO_ERROR;
    }

    if (fclose(stream) != 0) {
        return RESULT_EXPORT_IO_ERROR;
    }

    return RESULT_EXPORT_OK;
}

static ResultExportStatus write_scores_csv(
    const char *path,
    const double *scores,
    size_t score_count
)
{
    FILE *stream = fopen(path, "w");

    if (stream == NULL) {
        return RESULT_EXPORT_IO_ERROR;
    }

    if (
        fputs(
            "comparison_index,distance\n",
            stream
        ) == EOF
    ) {
        (void)fclose(stream);
        return RESULT_EXPORT_IO_ERROR;
    }

    for (
        size_t index = 0U;
        index < score_count;
        ++index
    ) {
        if (
            fprintf(
                stream,
                "%zu,%.17g\n",
                index,
                scores[index]
            ) < 0
        ) {
            (void)fclose(stream);
            return RESULT_EXPORT_IO_ERROR;
        }
    }

    if (fclose(stream) != 0) {
        return RESULT_EXPORT_IO_ERROR;
    }

    return RESULT_EXPORT_OK;
}

ResultExportStatus export_verification_result_csv(
    const VerificationExportMetadata *metadata,
    const VerificationExperimentResult *result,
    const char *summary_path,
    const char *genuine_scores_path,
    const char *impostor_scores_path
)
{
    if (
        !path_is_valid(summary_path) ||
        !path_is_valid(genuine_scores_path) ||
        !path_is_valid(impostor_scores_path) ||
        !metadata_is_valid(metadata)
    ) {
        return RESULT_EXPORT_INVALID_ARGUMENT;
    }

    if (!result_is_valid(result)) {
        return RESULT_EXPORT_INVALID_DATA;
    }

    ResultExportStatus status =
        write_summary_csv(
            summary_path,
            metadata,
            result
        );

    if (status != RESULT_EXPORT_OK) {
        return status;
    }

    status =
        write_scores_csv(
            genuine_scores_path,
            result->genuine_scores,
            result->genuine_comparisons
        );

    if (status != RESULT_EXPORT_OK) {
        return status;
    }

    status =
        write_scores_csv(
            impostor_scores_path,
            result->impostor_scores,
            result->impostor_comparisons
        );

    if (status != RESULT_EXPORT_OK) {
        return status;
    }

    return RESULT_EXPORT_OK;
}
