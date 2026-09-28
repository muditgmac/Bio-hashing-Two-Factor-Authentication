#include "preprocessing.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>

#define EPSILON 1e-12

static int nearly_equal(
    double first,
    double second
)
{
    return fabs(first - second) <= EPSILON;
}

static void test_sample_centering(void)
{
    const double input[6] = {
        1.0, 2.0, 3.0,
        4.0, 5.0, 6.0
    };

    double output[6] = {0.0};

    assert(
        sample_center_features(
            input,
            2U,
            3U,
            output
        ) == PREPROCESSING_OK
    );

    const double expected[6] = {
        -1.0, 0.0, 1.0,
        -1.0, 0.0, 1.0
    };

    for (size_t index = 0U; index < 6U; ++index) {
        assert(
            nearly_equal(
                output[index],
                expected[index]
            )
        );
    }
}

static void test_sample_centering_in_place(void)
{
    double values[6] = {
        2.0, 4.0, 6.0,
        10.0, 10.0, 10.0
    };

    assert(
        sample_center_features(
            values,
            2U,
            3U,
            values
        ) == PREPROCESSING_OK
    );

    const double expected[6] = {
        -2.0, 0.0, 2.0,
        0.0, 0.0, 0.0
    };

    for (size_t index = 0U; index < 6U; ++index) {
        assert(
            nearly_equal(
                values[index],
                expected[index]
            )
        );
    }
}

static void test_population_fit_and_apply(void)
{
    const double development[6] = {
        1.0, 2.0,
        3.0, 4.0,
        5.0, 6.0
    };

    PopulationCenteringModel model = {0};

    assert(
        fit_population_centering(
            development,
            3U,
            2U,
            &model
        ) == PREPROCESSING_OK
    );

    assert(model.feature_count == 2U);
    assert(nearly_equal(model.population_mean[0], 3.0));
    assert(nearly_equal(model.population_mean[1], 4.0));

    const double evaluation[4] = {
        7.0, 8.0,
        1.0, 2.0
    };

    double output[4] = {0.0};

    assert(
        apply_population_centering(
            &model,
            evaluation,
            2U,
            2U,
            output
        ) == PREPROCESSING_OK
    );

    const double expected[4] = {
        4.0, 4.0,
        -2.0, -2.0
    };

    for (size_t index = 0U; index < 4U; ++index) {
        assert(
            nearly_equal(
                output[index],
                expected[index]
            )
        );
    }

    population_centering_model_free(&model);
}

static void test_population_apply_in_place(void)
{
    const double development[6] = {
        2.0, 10.0,
        4.0, 20.0,
        6.0, 30.0
    };

    PopulationCenteringModel model = {0};

    assert(
        fit_population_centering(
            development,
            3U,
            2U,
            &model
        ) == PREPROCESSING_OK
    );

    double evaluation[4] = {
        8.0, 40.0,
        4.0, 20.0
    };

    assert(
        apply_population_centering(
            &model,
            evaluation,
            2U,
            2U,
            evaluation
        ) == PREPROCESSING_OK
    );

    assert(nearly_equal(evaluation[0], 4.0));
    assert(nearly_equal(evaluation[1], 20.0));
    assert(nearly_equal(evaluation[2], 0.0));
    assert(nearly_equal(evaluation[3], 0.0));

    population_centering_model_free(&model);
}

static void test_population_training_mean_becomes_zero(void)
{
    const double development[12] = {
         1.0,  2.0,  3.0,
         2.0,  4.0,  6.0,
         3.0,  6.0,  9.0,
         4.0,  8.0, 12.0
    };

    PopulationCenteringModel model = {0};
    double centered[12] = {0.0};

    assert(
        fit_population_centering(
            development,
            4U,
            3U,
            &model
        ) == PREPROCESSING_OK
    );

    assert(
        apply_population_centering(
            &model,
            development,
            4U,
            3U,
            centered
        ) == PREPROCESSING_OK
    );

    for (
        size_t feature = 0U;
        feature < 3U;
        ++feature
    ) {
        double sum = 0.0;

        for (
            size_t sample = 0U;
            sample < 4U;
            ++sample
        ) {
            sum += centered[
                sample * 3U +
                feature
            ];
        }

        assert(
            nearly_equal(
                sum / 4.0,
                0.0
            )
        );
    }

    population_centering_model_free(&model);
}

static void test_invalid_arguments_and_data(void)
{
    const double valid[2] = {
        1.0,
        2.0
    };

    double output[2] = {0.0};

    PopulationCenteringModel model = {0};

    assert(
        sample_center_features(
            NULL,
            1U,
            2U,
            output
        ) == PREPROCESSING_INVALID_ARGUMENT
    );

    assert(
        sample_center_features(
            valid,
            0U,
            2U,
            output
        ) == PREPROCESSING_INVALID_ARGUMENT
    );

    const double invalid[2] = {
        1.0,
        NAN
    };

    assert(
        sample_center_features(
            invalid,
            1U,
            2U,
            output
        ) == PREPROCESSING_INVALID_DATA
    );

    assert(
        fit_population_centering(
            invalid,
            1U,
            2U,
            &model
        ) == PREPROCESSING_INVALID_DATA
    );

    assert(
        apply_population_centering(
            &model,
            valid,
            1U,
            2U,
            output
        ) == PREPROCESSING_INVALID_ARGUMENT
    );

    population_centering_model_free(&model);
    population_centering_model_free(&model);
    population_centering_model_free(NULL);
}

static void test_failed_refit_preserves_model(void)
{
    const double valid[4] = {
        1.0, 3.0,
        5.0, 7.0
    };

    const double invalid[4] = {
        1.0, 3.0,
        NAN, 7.0
    };

    PopulationCenteringModel model = {0};

    assert(
        fit_population_centering(
            valid,
            2U,
            2U,
            &model
        ) == PREPROCESSING_OK
    );

    assert(nearly_equal(model.population_mean[0], 3.0));
    assert(nearly_equal(model.population_mean[1], 5.0));

    assert(
        fit_population_centering(
            invalid,
            2U,
            2U,
            &model
        ) == PREPROCESSING_INVALID_DATA
    );

    assert(model.feature_count == 2U);
    assert(nearly_equal(model.population_mean[0], 3.0));
    assert(nearly_equal(model.population_mean[1], 5.0));

    population_centering_model_free(&model);
}

int main(void)
{
    test_sample_centering();
    test_sample_centering_in_place();
    test_population_fit_and_apply();
    test_population_apply_in_place();
    test_population_training_mean_becomes_zero();
    test_invalid_arguments_and_data();
    test_failed_refit_preserves_model();

    printf("All preprocessing tests passed.\n");

    return 0;
}
