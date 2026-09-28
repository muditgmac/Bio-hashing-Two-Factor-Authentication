#include "dataset.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>

static void test_valid_dataset_with_header(void)
{
    Dataset dataset = {0};

    assert(
        dataset_load_csv(
            "tests/data/dataset_valid.csv",
            &dataset
        ) == DATASET_OK
    );

    assert(dataset.sample_count == 4U);
    assert(dataset.feature_count == 4U);

    assert(dataset.subject_ids[0] == 1U);
    assert(dataset.subject_ids[1] == 1U);
    assert(dataset.subject_ids[2] == 2U);
    assert(dataset.subject_ids[3] == 2U);

    assert(
        fabs(dataset.features[0] - 1.0) <
        1e-12
    );

    assert(
        fabs(dataset.features[15] - (-4.1)) <
        1e-12
    );

    dataset_free(&dataset);

    assert(dataset.sample_count == 0U);
    assert(dataset.feature_count == 0U);
    assert(dataset.subject_ids == NULL);
    assert(dataset.features == NULL);
}

static void test_dataset_without_header(void)
{
    Dataset dataset = {0};

    assert(
        dataset_load_csv(
            "tests/data/dataset_no_header.csv",
            &dataset
        ) == DATASET_OK
    );

    assert(dataset.sample_count == 2U);
    assert(dataset.feature_count == 3U);
    assert(dataset.subject_ids[0] == 7U);
    assert(dataset.subject_ids[1] == 8U);

    dataset_free(&dataset);
}

static void test_inconsistent_width_is_rejected(void)
{
    Dataset dataset = {0};

    assert(
        dataset_load_csv(
            "tests/data/dataset_bad_width.csv",
            &dataset
        ) == DATASET_FORMAT_ERROR
    );

    dataset_free(&dataset);
}

static void test_nonfinite_feature_is_rejected(void)
{
    Dataset dataset = {0};

    assert(
        dataset_load_csv(
            "tests/data/dataset_nonfinite.csv",
            &dataset
        ) == DATASET_NONFINITE_VALUE
    );

    dataset_free(&dataset);
}

static void test_missing_file_is_reported(void)
{
    Dataset dataset = {0};

    assert(
        dataset_load_csv(
            "tests/data/does_not_exist.csv",
            &dataset
        ) == DATASET_IO_ERROR
    );
}

static void test_invalid_arguments(void)
{
    Dataset dataset = {0};

    assert(
        dataset_load_csv(
            NULL,
            &dataset
        ) == DATASET_INVALID_ARGUMENT
    );

    assert(
        dataset_load_csv(
            "tests/data/dataset_valid.csv",
            NULL
        ) == DATASET_INVALID_ARGUMENT
    );
}

static void test_free_is_null_safe(void)
{
    dataset_free(NULL);

    Dataset dataset = {0};

    dataset_free(&dataset);

    assert(dataset.subject_ids == NULL);
    assert(dataset.features == NULL);
}

int main(void)
{
    test_valid_dataset_with_header();
    test_dataset_without_header();
    test_inconsistent_width_is_rejected();
    test_nonfinite_feature_is_rejected();
    test_missing_file_is_reported();
    test_invalid_arguments();
    test_free_is_null_safe();

    printf("All CSV dataset loader tests passed.\n");

    return 0;
}
