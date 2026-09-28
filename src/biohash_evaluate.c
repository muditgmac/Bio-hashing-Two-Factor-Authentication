#include "biohash.h"
#include "dataset.h"
#include "experiment.h"

#include <errno.h>
#include <inttypes.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

    verification_experiment_result_free(&result);
    dataset_free(&dataset);

    return EXIT_SUCCESS;
}
