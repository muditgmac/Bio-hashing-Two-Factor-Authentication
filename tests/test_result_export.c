#include "result_export.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#define SUMMARY_PATH \
    "build/test_result_export_summary.csv"

#define GENUINE_PATH \
    "build/test_result_export_genuine.csv"

#define IMPOSTOR_PATH \
    "build/test_result_export_impostor.csv"

#define ERROR_RATES_PATH \
    "build/test_result_export_error_rates.csv"

static VerificationExportMetadata test_metadata(void)
{
    VerificationExportMetadata metadata = {
        .input_path = "examples/demo,\"quoted\".csv",
        .feature_count = 4U,
        .hash_length = 3U,
        .preprocessing_mode =
            EXPERIMENT_PREPROCESSING_SAMPLE_CENTER,
        .biohash_config = {
            .p = 499U,
            .q = 547U,
            .seed = 12345U,
            .threshold = 0.0,
            .orthogonality_tolerance = 1e-12
        }
    };

    return metadata;
}

static VerificationExperimentResult test_result(
    double *genuine_scores,
    double *impostor_scores
)
{
    VerificationExperimentResult result = {
        .sample_count = 4U,
        .genuine_comparisons = 2U,
        .impostor_comparisons = 4U,

        .mean_genuine_distance = 0.125,
        .mean_impostor_distance = 0.75,

        .equal_error_rate = {
            .threshold = 0.5,
            .eer = 0.25,
            .fmr = 0.25,
            .fnmr = 0.25
        },

        .genuine_scores = genuine_scores,
        .impostor_scores = impostor_scores
    };

    return result;
}

static void read_file_text(
    const char *path,
    char *buffer,
    size_t capacity
)
{
    assert(capacity > 0U);

    FILE *stream = fopen(path, "r");

    assert(stream != NULL);

    const size_t bytes_read =
        fread(
            buffer,
            1U,
            capacity - 1U,
            stream
        );

    assert(!ferror(stream));

    buffer[bytes_read] = '\0';

    assert(fclose(stream) == 0);
}

static void remove_test_files(void)
{
    (void)remove(SUMMARY_PATH);
    (void)remove(GENUINE_PATH);
    (void)remove(IMPOSTOR_PATH);
    (void)remove(ERROR_RATES_PATH);
}

static void test_export_writes_expected_files(void)
{
    double genuine_scores[2U] = {
        0.0,
        0.25
    };

    double impostor_scores[4U] = {
        0.5,
        0.75,
        0.75,
        1.0
    };

    const VerificationExportMetadata metadata =
        test_metadata();

    const VerificationExperimentResult result =
        test_result(
            genuine_scores,
            impostor_scores
        );

    remove_test_files();

    assert(
        export_verification_result_csv(
            &metadata,
            &result,
            SUMMARY_PATH,
            GENUINE_PATH,
            IMPOSTOR_PATH
        ) == RESULT_EXPORT_OK
    );

    char summary[4096U];

    read_file_text(
        SUMMARY_PATH,
        summary,
        sizeof(summary)
    );

    assert(
        strstr(
            summary,
            "input_file,sample_count,feature_count,"
            "hash_length,preprocessing,"
        ) != NULL
    );

    /*
     * Verify proper CSV escaping for comma and quote characters.
     */
    assert(
        strstr(
            summary,
            "\"examples/demo,\"\"quoted\"\".csv\""
        ) != NULL
    );

    assert(
        strstr(
            summary,
            ",4,4,3,sample-center,499,547,12345,"
        ) != NULL
    );

    assert(
        strstr(
            summary,
            ",2,4,0.125,0.75,0.5,0.25,0.25,0.25"
        ) != NULL
    );

    char genuine[1024U];

    read_file_text(
        GENUINE_PATH,
        genuine,
        sizeof(genuine)
    );

    assert(
        strcmp(
            genuine,
            "comparison_index,distance\n"
            "0,0\n"
            "1,0.25\n"
        ) == 0
    );

    char impostor[1024U];

    read_file_text(
        IMPOSTOR_PATH,
        impostor,
        sizeof(impostor)
    );

    assert(
        strcmp(
            impostor,
            "comparison_index,distance\n"
            "0,0.5\n"
            "1,0.75\n"
            "2,0.75\n"
            "3,1\n"
        ) == 0
    );

    remove_test_files();
}


static void test_error_rate_sweep_export(void)
{
    ErrorRates points[4U] = {
        {
            .threshold = 0.0,
            .fmr = 1.0,
            .fnmr = 0.0
        },
        {
            .threshold = 0.25,
            .fmr = 0.5,
            .fnmr = 0.25
        },
        {
            .threshold = 0.5,
            .fmr = 0.25,
            .fnmr = 0.5
        },
        {
            .threshold = 1.0,
            .fmr = 0.0,
            .fnmr = 1.0
        }
    };

    ErrorRateSweepResult sweep = {
        .point_count = 4U,
        .points = points
    };

    (void)remove(ERROR_RATES_PATH);

    assert(
        export_error_rate_sweep_csv(
            &sweep,
            ERROR_RATES_PATH
        ) == RESULT_EXPORT_OK
    );

    char contents[1024U];

    read_file_text(
        ERROR_RATES_PATH,
        contents,
        sizeof(contents)
    );

    assert(
        strcmp(
            contents,
            "threshold,fmr,fnmr\n"
            "0,1,0\n"
            "0.25,0.5,0.25\n"
            "0.5,0.25,0.5\n"
            "1,0,1\n"
        ) == 0
    );

    assert(
        export_error_rate_sweep_csv(
            NULL,
            ERROR_RATES_PATH
        ) == RESULT_EXPORT_INVALID_DATA
    );

    assert(
        export_error_rate_sweep_csv(
            &sweep,
            NULL
        ) == RESULT_EXPORT_INVALID_ARGUMENT
    );

    assert(
        export_error_rate_sweep_csv(
            &sweep,
            ""
        ) == RESULT_EXPORT_INVALID_ARGUMENT
    );

    points[2].fmr = 1.25;

    assert(
        export_error_rate_sweep_csv(
            &sweep,
            ERROR_RATES_PATH
        ) == RESULT_EXPORT_INVALID_DATA
    );

    (void)remove(ERROR_RATES_PATH);
}

static void test_invalid_arguments_are_rejected(void)
{
    double genuine_scores[2U] = {
        0.0,
        0.25
    };

    double impostor_scores[4U] = {
        0.5,
        0.75,
        0.75,
        1.0
    };

    const VerificationExportMetadata metadata =
        test_metadata();

    const VerificationExperimentResult result =
        test_result(
            genuine_scores,
            impostor_scores
        );

    assert(
        export_verification_result_csv(
            NULL,
            &result,
            SUMMARY_PATH,
            GENUINE_PATH,
            IMPOSTOR_PATH
        ) == RESULT_EXPORT_INVALID_ARGUMENT
    );

    assert(
        export_verification_result_csv(
            &metadata,
            &result,
            NULL,
            GENUINE_PATH,
            IMPOSTOR_PATH
        ) == RESULT_EXPORT_INVALID_ARGUMENT
    );

    assert(
        export_verification_result_csv(
            &metadata,
            &result,
            "",
            GENUINE_PATH,
            IMPOSTOR_PATH
        ) == RESULT_EXPORT_INVALID_ARGUMENT
    );
}

static void test_invalid_preprocessing_mode_is_rejected(void)
{
    double genuine_scores[2U] = {
        0.0,
        0.25
    };

    double impostor_scores[4U] = {
        0.5,
        0.75,
        0.75,
        1.0
    };

    VerificationExportMetadata invalid_metadata =
        test_metadata();

    invalid_metadata.preprocessing_mode =
        (ExperimentPreprocessingMode)999;

    const VerificationExperimentResult result =
        test_result(
            genuine_scores,
            impostor_scores
        );

    assert(
        export_verification_result_csv(
            &invalid_metadata,
            &result,
            SUMMARY_PATH,
            GENUINE_PATH,
            IMPOSTOR_PATH
        ) == RESULT_EXPORT_INVALID_ARGUMENT
    );
}

static void test_invalid_score_data_is_rejected(void)
{
    double genuine_scores[2U] = {
        0.0,
        1.25
    };

    double impostor_scores[4U] = {
        0.5,
        0.75,
        0.75,
        1.0
    };

    const VerificationExportMetadata metadata =
        test_metadata();

    const VerificationExperimentResult result =
        test_result(
            genuine_scores,
            impostor_scores
        );

    assert(
        export_verification_result_csv(
            &metadata,
            &result,
            SUMMARY_PATH,
            GENUINE_PATH,
            IMPOSTOR_PATH
        ) == RESULT_EXPORT_INVALID_DATA
    );
}

static void test_io_failure_is_reported(void)
{
    double genuine_scores[2U] = {
        0.0,
        0.25
    };

    double impostor_scores[4U] = {
        0.5,
        0.75,
        0.75,
        1.0
    };

    const VerificationExportMetadata metadata =
        test_metadata();

    const VerificationExperimentResult result =
        test_result(
            genuine_scores,
            impostor_scores
        );

    assert(
        export_verification_result_csv(
            &metadata,
            &result,
            "build/nonexistent-directory/summary.csv",
            GENUINE_PATH,
            IMPOSTOR_PATH
        ) == RESULT_EXPORT_IO_ERROR
    );
}

int main(void)
{
    test_export_writes_expected_files();
    test_error_rate_sweep_export();
    test_invalid_arguments_are_rejected();
    test_invalid_preprocessing_mode_is_rejected();
    test_invalid_score_data_is_rejected();
    test_io_failure_is_reported();

    remove_test_files();

    printf(
        "All verification result export tests passed.\n"
    );

    return 0;
}
