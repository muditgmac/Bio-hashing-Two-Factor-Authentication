#include "biohash.h"
#include "dataset.h"
#include "experiment.h"
#include "result_export.h"

#include <errno.h>
#include <inttypes.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

static void print_usage(
    FILE *stream,
    const char *program
)
{
    fprintf(
        stream,
        "Usage:\n"
        "  %s --input FILE --hash-length N [options]\n"
        "\n"
        "Required arguments:\n"
        "  --input FILE          CSV biometric feature dataset\n"
        "  --hash-length N       Number of BioHash bits\n"
        "\n"
        "BioHash token options:\n"
        "  --p N                 First Blum prime (default: 499)\n"
        "  --q N                 Second Blum prime (default: 547)\n"
        "  --seed N              BBS seed (default: 12345)\n"
        "  --threshold X         Quantization threshold (default: 0.0)\n"
        "  --tolerance X         Orthogonality tolerance (default: 1e-12)\n"
        "\n"
        "Result export:\n"
        "  --output-dir DIR     Export summary and score CSV files\n"
        "\n"
        "Other options:\n"
        "  --help                Show this help message\n",
        program
    );
}

static int parse_size_value(
    const char *text,
    size_t *result
)
{
    if (
        text == NULL ||
        result == NULL ||
        *text == '\0' ||
        *text == '-'
    ) {
        return 0;
    }

    errno = 0;

    char *end = NULL;

    const uintmax_t value =
        strtoumax(
            text,
            &end,
            10
        );

    if (
        errno == ERANGE ||
        end == text ||
        *end != '\0' ||
        value > SIZE_MAX
    ) {
        return 0;
    }

    *result = (size_t)value;

    return 1;
}

static int parse_uint64_value(
    const char *text,
    uint64_t *result
)
{
    if (
        text == NULL ||
        result == NULL ||
        *text == '\0' ||
        *text == '-'
    ) {
        return 0;
    }

    errno = 0;

    char *end = NULL;

    const uintmax_t value =
        strtoumax(
            text,
            &end,
            10
        );

    if (
        errno == ERANGE ||
        end == text ||
        *end != '\0' ||
        value > UINT64_MAX
    ) {
        return 0;
    }

    *result = (uint64_t)value;

    return 1;
}

static int parse_double_value(
    const char *text,
    double *result
)
{
    if (
        text == NULL ||
        result == NULL ||
        *text == '\0'
    ) {
        return 0;
    }

    errno = 0;

    char *end = NULL;

    const double value =
        strtod(
            text,
            &end
        );

    if (
        errno == ERANGE ||
        end == text ||
        *end != '\0' ||
        !isfinite(value)
    ) {
        return 0;
    }

    *result = value;

    return 1;
}

static int path_is_directory(
    const char *path
)
{
    struct stat information;

    if (
        path == NULL ||
        stat(path, &information) != 0
    ) {
        return 0;
    }

    return S_ISDIR(information.st_mode) ? 1 : 0;
}

static int create_directory_if_needed(
    const char *path
)
{
    if (
        path == NULL ||
        *path == '\0'
    ) {
        return 0;
    }

    if (mkdir(path, 0777) == 0) {
        return 1;
    }

    if (
        errno == EEXIST &&
        path_is_directory(path)
    ) {
        return 1;
    }

    return 0;
}

static int ensure_directory_tree(
    const char *path
)
{
    if (
        path == NULL ||
        *path == '\0'
    ) {
        return 0;
    }

    size_t length = strlen(path);

    if (length == SIZE_MAX) {
        return 0;
    }

    char *copy = malloc(length + 1U);

    if (copy == NULL) {
        return 0;
    }

    memcpy(
        copy,
        path,
        length + 1U
    );

    while (
        length > 1U &&
        copy[length - 1U] == '/'
    ) {
        copy[length - 1U] = '\0';
        --length;
    }

    for (size_t index = 1U; index < length; ++index) {
        if (copy[index] != '/') {
            continue;
        }

        copy[index] = '\0';

        if (
            copy[0] != '\0' &&
            !create_directory_if_needed(copy)
        ) {
            free(copy);
            return 0;
        }

        copy[index] = '/';
    }

    const int success =
        create_directory_if_needed(copy);

    free(copy);

    return success;
}

static char *build_output_path(
    const char *directory,
    const char *filename
)
{
    if (
        directory == NULL ||
        filename == NULL ||
        *directory == '\0' ||
        *filename == '\0'
    ) {
        return NULL;
    }

    const size_t directory_length =
        strlen(directory);

    const size_t filename_length =
        strlen(filename);

    const int needs_separator =
        directory[directory_length - 1U] != '/';

    const size_t separator_length =
        needs_separator ? 1U : 0U;

    if (
        directory_length >
        SIZE_MAX -
            filename_length -
            separator_length -
            1U
    ) {
        return NULL;
    }

    const size_t total_length =
        directory_length +
        separator_length +
        filename_length;

    char *output_path =
        malloc(total_length + 1U);

    if (output_path == NULL) {
        return NULL;
    }

    size_t offset = 0U;

    memcpy(
        output_path,
        directory,
        directory_length
    );

    offset += directory_length;

    if (needs_separator) {
        output_path[offset] = '/';
        ++offset;
    }

    memcpy(
        output_path + offset,
        filename,
        filename_length
    );

    offset += filename_length;
    output_path[offset] = '\0';

    return output_path;
}

static const char *result_export_status_string(
    ResultExportStatus status
)
{
    switch (status) {
        case RESULT_EXPORT_OK:
            return "success";

        case RESULT_EXPORT_INVALID_ARGUMENT:
            return "invalid export argument";

        case RESULT_EXPORT_INVALID_DATA:
            return "invalid verification result data";

        case RESULT_EXPORT_IO_ERROR:
            return "I/O error";
    }

    return "unknown result-export error";
}

static const char *dataset_status_string(
    DatasetStatus status
)
{
    switch (status) {
        case DATASET_OK:
            return "success";

        case DATASET_INVALID_ARGUMENT:
            return "invalid argument";

        case DATASET_IO_ERROR:
            return "I/O error";

        case DATASET_FORMAT_ERROR:
            return "invalid CSV format";

        case DATASET_NONFINITE_VALUE:
            return "non-finite feature value";

        case DATASET_ALLOCATION_FAILURE:
            return "memory allocation failure";

        case DATASET_SIZE_OVERFLOW:
            return "dataset size overflow";
    }

    return "unknown dataset error";
}

static const char *experiment_status_string(
    ExperimentStatus status
)
{
    switch (status) {
        case EXPERIMENT_OK:
            return "success";

        case EXPERIMENT_INVALID_ARGUMENT:
            return "invalid experiment argument";

        case EXPERIMENT_ALLOCATION_FAILURE:
            return "memory allocation failure";

        case EXPERIMENT_BIOHASH_FAILURE:
            return "BioHash generation failure";

        case EXPERIMENT_MATCHER_FAILURE:
            return "template matcher failure";

        case EXPERIMENT_EVALUATION_FAILURE:
            return "biometric evaluation failure";

        case EXPERIMENT_INSUFFICIENT_COMPARISONS:
            return "insufficient genuine/impostor comparisons";
    }

    return "unknown experiment error";
}

int main(
    int argc,
    char **argv
)
{
    const char *input_path = NULL;
    const char *output_dir = NULL;

    size_t hash_length = 0U;
    int hash_length_was_set = 0;

    BioHashConfig config = {
        .p = 499U,
        .q = 547U,
        .seed = 12345U,
        .threshold = 0.0,
        .orthogonality_tolerance = 1e-12
    };

    if (argc <= 1) {
        print_usage(stderr, argv[0]);
        return EXIT_FAILURE;
    }

    for (int index = 1; index < argc; ++index) {
        if (
            strcmp(
                argv[index],
                "--help"
            ) == 0
        ) {
            print_usage(stdout, argv[0]);
            return EXIT_SUCCESS;
        }

        if (
            strcmp(
                argv[index],
                "--input"
            ) == 0
        ) {
            if (index + 1 >= argc) {
                fprintf(
                    stderr,
                    "Error: --input requires a file path.\n"
                );

                return EXIT_FAILURE;
            }

            ++index;
            input_path = argv[index];

            continue;
        }

        if (
            strcmp(
                argv[index],
                "--output-dir"
            ) == 0
        ) {
            if (
                index + 1 >= argc ||
                argv[index + 1][0] == '\0'
            ) {
                fprintf(
                    stderr,
                    "Error: --output-dir requires a directory path.\n"
                );

                return EXIT_FAILURE;
            }

            ++index;
            output_dir = argv[index];

            continue;
        }

        if (
            strcmp(
                argv[index],
                "--hash-length"
            ) == 0
        ) {
            if (
                index + 1 >= argc ||
                !parse_size_value(
                    argv[index + 1],
                    &hash_length
                ) ||
                hash_length == 0U
            ) {
                fprintf(
                    stderr,
                    "Error: --hash-length requires a positive integer.\n"
                );

                return EXIT_FAILURE;
            }

            ++index;
            hash_length_was_set = 1;

            continue;
        }

        if (
            strcmp(
                argv[index],
                "--p"
            ) == 0
        ) {
            if (
                index + 1 >= argc ||
                !parse_uint64_value(
                    argv[index + 1],
                    &config.p
                )
            ) {
                fprintf(
                    stderr,
                    "Error: --p requires an unsigned integer.\n"
                );

                return EXIT_FAILURE;
            }

            ++index;
            continue;
        }

        if (
            strcmp(
                argv[index],
                "--q"
            ) == 0
        ) {
            if (
                index + 1 >= argc ||
                !parse_uint64_value(
                    argv[index + 1],
                    &config.q
                )
            ) {
                fprintf(
                    stderr,
                    "Error: --q requires an unsigned integer.\n"
                );

                return EXIT_FAILURE;
            }

            ++index;
            continue;
        }

        if (
            strcmp(
                argv[index],
                "--seed"
            ) == 0
        ) {
            if (
                index + 1 >= argc ||
                !parse_uint64_value(
                    argv[index + 1],
                    &config.seed
                )
            ) {
                fprintf(
                    stderr,
                    "Error: --seed requires an unsigned integer.\n"
                );

                return EXIT_FAILURE;
            }

            ++index;
            continue;
        }

        if (
            strcmp(
                argv[index],
                "--threshold"
            ) == 0
        ) {
            if (
                index + 1 >= argc ||
                !parse_double_value(
                    argv[index + 1],
                    &config.threshold
                )
            ) {
                fprintf(
                    stderr,
                    "Error: --threshold requires a finite number.\n"
                );

                return EXIT_FAILURE;
            }

            ++index;
            continue;
        }

        if (
            strcmp(
                argv[index],
                "--tolerance"
            ) == 0
        ) {
            if (
                index + 1 >= argc ||
                !parse_double_value(
                    argv[index + 1],
                    &config.orthogonality_tolerance
                ) ||
                config.orthogonality_tolerance < 0.0
            ) {
                fprintf(
                    stderr,
                    "Error: --tolerance requires a non-negative finite number.\n"
                );

                return EXIT_FAILURE;
            }

            ++index;
            continue;
        }

        fprintf(
            stderr,
            "Error: unknown argument '%s'.\n\n",
            argv[index]
        );

        print_usage(stderr, argv[0]);

        return EXIT_FAILURE;
    }

    if (input_path == NULL) {
        fprintf(
            stderr,
            "Error: --input is required.\n\n"
        );

        print_usage(stderr, argv[0]);

        return EXIT_FAILURE;
    }

    if (!hash_length_was_set) {
        fprintf(
            stderr,
            "Error: --hash-length is required.\n\n"
        );

        print_usage(stderr, argv[0]);

        return EXIT_FAILURE;
    }

    Dataset dataset = {0};

    const DatasetStatus dataset_status =
        dataset_load_csv(
            input_path,
            &dataset
        );

    if (dataset_status != DATASET_OK) {
        fprintf(
            stderr,
            "Error loading dataset '%s': %s.\n",
            input_path,
            dataset_status_string(dataset_status)
        );

        dataset_free(&dataset);

        return EXIT_FAILURE;
    }

    if (hash_length > dataset.feature_count) {
        fprintf(
            stderr,
            "Error: hash length (%zu) exceeds feature count (%zu).\n",
            hash_length,
            dataset.feature_count
        );

        dataset_free(&dataset);

        return EXIT_FAILURE;
    }

    VerificationExperimentResult result = {0};

    const ExperimentStatus experiment_status =
        run_verification_experiment(
            dataset.features,
            dataset.subject_ids,
            dataset.sample_count,
            dataset.feature_count,
            hash_length,
            &config,
            &result
        );

    if (experiment_status != EXPERIMENT_OK) {
        fprintf(
            stderr,
            "Verification experiment failed: %s.\n",
            experiment_status_string(
                experiment_status
            )
        );

        dataset_free(&dataset);

        return EXIT_FAILURE;
    }

    printf(
        "\n"
        "BioHash Verification Experiment\n"
        "===============================\n"
        "\n"
        "Dataset\n"
        "-------\n"
        "Input file:                 %s\n"
        "Samples:                    %zu\n"
        "Features per sample:        %zu\n"
        "Hash length:                %zu\n"
        "\n"
        "Token configuration\n"
        "-------------------\n"
        "BBS p:                      %" PRIu64 "\n"
        "BBS q:                      %" PRIu64 "\n"
        "BBS seed:                   %" PRIu64 "\n"
        "Quantization threshold:     %.12g\n"
        "Orthogonality tolerance:    %.12g\n"
        "\n"
        "Comparisons\n"
        "-----------\n"
        "Genuine comparisons:        %zu\n"
        "Impostor comparisons:       %zu\n"
        "Mean genuine distance:      %.6f\n"
        "Mean impostor distance:     %.6f\n"
        "\n"
        "Estimated EER operating point\n"
        "-----------------------------\n"
        "Decision threshold:         %.6f\n"
        "FMR:                        %.6f\n"
        "FNMR:                       %.6f\n"
        "Estimated EER:              %.6f\n"
        "\n",
        input_path,
        result.sample_count,
        dataset.feature_count,
        hash_length,
        config.p,
        config.q,
        config.seed,
        config.threshold,
        config.orthogonality_tolerance,
        result.genuine_comparisons,
        result.impostor_comparisons,
        result.mean_genuine_distance,
        result.mean_impostor_distance,
        result.equal_error_rate.threshold,
        result.equal_error_rate.fmr,
        result.equal_error_rate.fnmr,
        result.equal_error_rate.eer
    );

    if (output_dir != NULL) {
        if (!ensure_directory_tree(output_dir)) {
            fprintf(
                stderr,
                "Error: unable to create output directory '%s'.\n",
                output_dir
            );

            verification_experiment_result_free(&result);
            dataset_free(&dataset);

            return EXIT_FAILURE;
        }

        char *summary_path =
            build_output_path(
                output_dir,
                "summary.csv"
            );

        char *genuine_scores_path =
            build_output_path(
                output_dir,
                "genuine_scores.csv"
            );

        char *impostor_scores_path =
            build_output_path(
                output_dir,
                "impostor_scores.csv"
            );

        if (
            summary_path == NULL ||
            genuine_scores_path == NULL ||
            impostor_scores_path == NULL
        ) {
            fprintf(
                stderr,
                "Error: unable to allocate result-export paths.\n"
            );

            free(summary_path);
            free(genuine_scores_path);
            free(impostor_scores_path);

            verification_experiment_result_free(&result);
            dataset_free(&dataset);

            return EXIT_FAILURE;
        }

        const VerificationExportMetadata metadata = {
            .input_path = input_path,
            .feature_count = dataset.feature_count,
            .hash_length = hash_length,
            .biohash_config = config
        };

        const ResultExportStatus export_status =
            export_verification_result_csv(
                &metadata,
                &result,
                summary_path,
                genuine_scores_path,
                impostor_scores_path
            );

        if (export_status != RESULT_EXPORT_OK) {
            fprintf(
                stderr,
                "Error exporting verification results: %s.\n",
                result_export_status_string(
                    export_status
                )
            );

            free(summary_path);
            free(genuine_scores_path);
            free(impostor_scores_path);

            verification_experiment_result_free(&result);
            dataset_free(&dataset);

            return EXIT_FAILURE;
        }

        printf(
            "Results exported to:          %s\n"
            "  summary:                    %s\n"
            "  genuine scores:             %s\n"
            "  impostor scores:            %s\n"
            "\n",
            output_dir,
            summary_path,
            genuine_scores_path,
            impostor_scores_path
        );

        free(summary_path);
        free(genuine_scores_path);
        free(impostor_scores_path);
    }

    verification_experiment_result_free(&result);
    dataset_free(&dataset);

    return EXIT_SUCCESS;
}
