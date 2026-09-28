#include "dataset.h"

#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *skip_whitespace(
    const char *text
)
{
    while (
        text != NULL &&
        *text != '\0' &&
        isspace((unsigned char)*text)
    ) {
        ++text;
    }

    return text;
}

static int line_is_blank(
    const char *line
)
{
    if (line == NULL) {
        return 1;
    }

    const char *cursor = skip_whitespace(line);

    return *cursor == '\0';
}

static int line_has_supported_header(
    const char *line
)
{
    if (line == NULL) {
        return 0;
    }

    const char *cursor = skip_whitespace(line);

    const char *comma = strchr(cursor, ',');

    if (comma == NULL) {
        return 0;
    }

    const char *end = comma;

    while (
        end > cursor &&
        isspace((unsigned char)*(end - 1))
    ) {
        --end;
    }

    const size_t length =
        (size_t)(end - cursor);

    if (
        length == strlen("subject_id") &&
        strncmp(
            cursor,
            "subject_id",
            length
        ) == 0
    ) {
        return 1;
    }

    if (
        length == strlen("subject") &&
        strncmp(
            cursor,
            "subject",
            length
        ) == 0
    ) {
        return 1;
    }

    if (
        length == strlen("id") &&
        strncmp(
            cursor,
            "id",
            length
        ) == 0
    ) {
        return 1;
    }

    return 0;
}

static int read_line(
    FILE *file,
    char **buffer,
    size_t *capacity
)
{
    if (
        file == NULL ||
        buffer == NULL ||
        capacity == NULL
    ) {
        return -1;
    }

    if (*buffer == NULL) {
        *capacity = 256U;
        *buffer = malloc(*capacity);

        if (*buffer == NULL) {
            return -1;
        }
    }

    size_t length = 0U;
    int character = 0;

    while (
        (character = fgetc(file)) != EOF
    ) {
        if (character == '\n') {
            break;
        }

        if (length + 1U >= *capacity) {
            if (*capacity > SIZE_MAX / 2U) {
                return -1;
            }

            const size_t new_capacity =
                *capacity * 2U;

            char *resized =
                realloc(
                    *buffer,
                    new_capacity
                );

            if (resized == NULL) {
                return -1;
            }

            *buffer = resized;
            *capacity = new_capacity;
        }

        (*buffer)[length] =
            (char)character;

        ++length;
    }

    if (
        character == EOF &&
        ferror(file)
    ) {
        return -2;
    }

    if (
        character == EOF &&
        length == 0U
    ) {
        return 0;
    }

    if (
        length > 0U &&
        (*buffer)[length - 1U] == '\r'
    ) {
        --length;
    }

    (*buffer)[length] = '\0';

    return 1;
}

static DatasetStatus append_feature(
    double **features,
    size_t *count,
    size_t *capacity,
    double value
)
{
    if (
        features == NULL ||
        count == NULL ||
        capacity == NULL
    ) {
        return DATASET_INVALID_ARGUMENT;
    }

    if (*count == *capacity) {
        size_t new_capacity =
            *capacity == 0U
                ? 8U
                : *capacity * 2U;

        if (
            *capacity != 0U &&
            new_capacity < *capacity
        ) {
            return DATASET_SIZE_OVERFLOW;
        }

        if (
            new_capacity >
            SIZE_MAX / sizeof(double)
        ) {
            return DATASET_SIZE_OVERFLOW;
        }

        double *resized =
            realloc(
                *features,
                new_capacity * sizeof(double)
            );

        if (resized == NULL) {
            return DATASET_ALLOCATION_FAILURE;
        }

        *features = resized;
        *capacity = new_capacity;
    }

    (*features)[*count] = value;
    ++(*count);

    return DATASET_OK;
}

static DatasetStatus parse_data_row(
    const char *line,
    size_t *subject_id,
    double **row_features,
    size_t *row_feature_count
)
{
    if (
        line == NULL ||
        subject_id == NULL ||
        row_features == NULL ||
        row_feature_count == NULL
    ) {
        return DATASET_INVALID_ARGUMENT;
    }

    *row_features = NULL;
    *row_feature_count = 0U;

    const char *cursor =
        skip_whitespace(line);

    if (
        *cursor == '\0' ||
        *cursor == '-'
    ) {
        return DATASET_FORMAT_ERROR;
    }

    errno = 0;

    char *end_pointer = NULL;

    const unsigned long long parsed_subject =
        strtoull(
            cursor,
            &end_pointer,
            10
        );

    if (
        end_pointer == cursor ||
        errno == ERANGE
    ) {
        return DATASET_FORMAT_ERROR;
    }

    if (parsed_subject > SIZE_MAX) {
        return DATASET_SIZE_OVERFLOW;
    }

    cursor =
        skip_whitespace(end_pointer);

    if (*cursor != ',') {
        return DATASET_FORMAT_ERROR;
    }

    ++cursor;

    double *features = NULL;
    size_t feature_count = 0U;
    size_t feature_capacity = 0U;

    for (;;) {
        cursor =
            skip_whitespace(cursor);

        if (*cursor == '\0') {
            free(features);
            return DATASET_FORMAT_ERROR;
        }

        errno = 0;
        end_pointer = NULL;

        const double value =
            strtod(
                cursor,
                &end_pointer
            );

        if (
            end_pointer == cursor ||
            errno == ERANGE
        ) {
            free(features);
            return DATASET_FORMAT_ERROR;
        }

        if (!isfinite(value)) {
            free(features);
            return DATASET_NONFINITE_VALUE;
        }

        DatasetStatus status =
            append_feature(
                &features,
                &feature_count,
                &feature_capacity,
                value
            );

        if (status != DATASET_OK) {
            free(features);
            return status;
        }

        cursor =
            skip_whitespace(end_pointer);

        if (*cursor == '\0') {
            break;
        }

        if (*cursor != ',') {
            free(features);
            return DATASET_FORMAT_ERROR;
        }

        ++cursor;
    }

    if (feature_count == 0U) {
        free(features);
        return DATASET_FORMAT_ERROR;
    }

    *subject_id =
        (size_t)parsed_subject;

    *row_features = features;
    *row_feature_count = feature_count;

    return DATASET_OK;
}

void dataset_free(
    Dataset *dataset
)
{
    if (dataset == NULL) {
        return;
    }

    free(dataset->subject_ids);
    free(dataset->features);

    dataset->sample_count = 0U;
    dataset->feature_count = 0U;
    dataset->subject_ids = NULL;
    dataset->features = NULL;
}

DatasetStatus dataset_load_csv(
    const char *path,
    Dataset *result
)
{
    if (
        path == NULL ||
        result == NULL
    ) {
        return DATASET_INVALID_ARGUMENT;
    }

    FILE *file = fopen(path, "r");

    if (file == NULL) {
        return DATASET_IO_ERROR;
    }

    Dataset loaded = {0};

    char *line = NULL;
    size_t line_capacity = 0U;

    size_t sample_capacity = 0U;
    int first_nonblank_line = 1;

    DatasetStatus final_status =
        DATASET_OK;

    for (;;) {
        const int read_status =
            read_line(
                file,
                &line,
                &line_capacity
            );

        if (read_status == 0) {
            break;
        }

        if (read_status == -1) {
            final_status =
                DATASET_ALLOCATION_FAILURE;
            break;
        }

        if (read_status == -2) {
            final_status =
                DATASET_IO_ERROR;
            break;
        }

        if (line_is_blank(line)) {
            continue;
        }

        if (
            first_nonblank_line &&
            line_has_supported_header(line)
        ) {
            first_nonblank_line = 0;
            continue;
        }

        first_nonblank_line = 0;

        size_t subject_id = 0U;
        double *row_features = NULL;
        size_t row_feature_count = 0U;

        DatasetStatus row_status =
            parse_data_row(
                line,
                &subject_id,
                &row_features,
                &row_feature_count
            );

        if (row_status != DATASET_OK) {
            free(row_features);
            final_status = row_status;
            break;
        }

        if (loaded.sample_count == 0U) {
            loaded.feature_count =
                row_feature_count;
        } else if (
            row_feature_count !=
            loaded.feature_count
        ) {
            free(row_features);

            final_status =
                DATASET_FORMAT_ERROR;

            break;
        }

        if (
            loaded.sample_count ==
            sample_capacity
        ) {
            const size_t new_capacity =
                sample_capacity == 0U
                    ? 16U
                    : sample_capacity * 2U;

            if (
                sample_capacity != 0U &&
                new_capacity < sample_capacity
            ) {
                free(row_features);

                final_status =
                    DATASET_SIZE_OVERFLOW;

                break;
            }

            if (
                new_capacity >
                SIZE_MAX / sizeof(size_t)
            ) {
                free(row_features);

                final_status =
                    DATASET_SIZE_OVERFLOW;

                break;
            }

            if (
                loaded.feature_count > 0U &&
                new_capacity >
                    SIZE_MAX /
                    loaded.feature_count
            ) {
                free(row_features);

                final_status =
                    DATASET_SIZE_OVERFLOW;

                break;
            }

            const size_t feature_elements =
                new_capacity *
                loaded.feature_count;

            if (
                feature_elements >
                SIZE_MAX / sizeof(double)
            ) {
                free(row_features);

                final_status =
                    DATASET_SIZE_OVERFLOW;

                break;
            }

            size_t *new_subject_ids =
                realloc(
                    loaded.subject_ids,
                    new_capacity *
                    sizeof(size_t)
                );

            if (new_subject_ids == NULL) {
                free(row_features);

                final_status =
                    DATASET_ALLOCATION_FAILURE;

                break;
            }

            loaded.subject_ids =
                new_subject_ids;

            double *new_features =
                realloc(
                    loaded.features,
                    feature_elements *
                    sizeof(double)
                );

            if (new_features == NULL) {
                free(row_features);

                final_status =
                    DATASET_ALLOCATION_FAILURE;

                break;
            }

            loaded.features =
                new_features;

            sample_capacity =
                new_capacity;
        }

        loaded.subject_ids[
            loaded.sample_count
        ] = subject_id;

        memcpy(
            &loaded.features[
                loaded.sample_count *
                loaded.feature_count
            ],
            row_features,
            loaded.feature_count *
                sizeof(double)
        );

        ++loaded.sample_count;

        free(row_features);
    }

    free(line);

    if (fclose(file) != 0) {
        if (final_status == DATASET_OK) {
            final_status =
                DATASET_IO_ERROR;
        }
    }

    if (
        final_status == DATASET_OK &&
        (
            loaded.sample_count == 0U ||
            loaded.feature_count == 0U
        )
    ) {
        final_status =
            DATASET_FORMAT_ERROR;
    }

    if (final_status != DATASET_OK) {
        dataset_free(&loaded);
        return final_status;
    }

    *result = loaded;

    return DATASET_OK;
}
