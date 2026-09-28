#include "dct.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>

#define TOLERANCE 1e-10

static int approximately_equal(
    double left,
    double right,
    double tolerance
)
{
    return fabs(left - right) <= tolerance;
}

static double vector_energy(
    const double *values,
    size_t length
)
{
    double energy = 0.0;

    for (size_t index = 0U; index < length; ++index) {
        energy += values[index] * values[index];
    }

    return energy;
}

static void test_constant_vector(void)
{
    const double input[] = {
        1.0,
        1.0,
        1.0,
        1.0
    };

    double output[4];

    const DCTStatus status =
        dct_ii_orthonormal(input, output, 4U);

    assert(status == DCT_OK);

    assert(
        approximately_equal(
            output[0],
            2.0,
            TOLERANCE
        )
    );

    assert(fabs(output[1]) < TOLERANCE);
    assert(fabs(output[2]) < TOLERANCE);
    assert(fabs(output[3]) < TOLERANCE);
}

static void test_known_vector(void)
{
    const double input[] = {
        1.0,
        2.0,
        3.0,
        4.0
    };

    const double expected[] = {
        5.0,
        -2.230442497387663,
        0.0,
        -0.158512667781108
    };

    double output[4];

    assert(
        dct_ii_orthonormal(
            input,
            output,
            4U
        ) == DCT_OK
    );

    for (size_t index = 0U; index < 4U; ++index) {
        assert(
            approximately_equal(
                output[index],
                expected[index],
                TOLERANCE
            )
        );
    }
}

static void test_single_element_vector(void)
{
    const double input[] = {7.5};
    double output[1];

    assert(
        dct_ii_orthonormal(
            input,
            output,
            1U
        ) == DCT_OK
    );

    assert(
        approximately_equal(
            output[0],
            7.5,
            TOLERANCE
        )
    );
}

static void test_energy_is_preserved(void)
{
    const double input[] = {
        2.5,
        -1.0,
        3.25,
        0.5,
        -4.0,
        1.75
    };

    double output[6];

    assert(
        dct_ii_orthonormal(
            input,
            output,
            6U
        ) == DCT_OK
    );

    assert(
        approximately_equal(
            vector_energy(input, 6U),
            vector_energy(output, 6U),
            1e-9
        )
    );
}

static void test_in_place_operation(void)
{
    double values[] = {
        1.0,
        2.0,
        3.0,
        4.0
    };

    const double expected[] = {
        5.0,
        -2.230442497387663,
        0.0,
        -0.158512667781108
    };

    assert(
        dct_ii_orthonormal(
            values,
            values,
            4U
        ) == DCT_OK
    );

    for (size_t index = 0U; index < 4U; ++index) {
        assert(
            approximately_equal(
                values[index],
                expected[index],
                TOLERANCE
            )
        );
    }
}

static void test_invalid_arguments(void)
{
    double values[4] = {
        1.0,
        2.0,
        3.0,
        4.0
    };

    assert(
        dct_ii_orthonormal(
            NULL,
            values,
            4U
        ) == DCT_INVALID_ARGUMENT
    );

    assert(
        dct_ii_orthonormal(
            values,
            NULL,
            4U
        ) == DCT_INVALID_ARGUMENT
    );

    assert(
        dct_ii_orthonormal(
            values,
            values,
            0U
        ) == DCT_INVALID_ARGUMENT
    );
}

static void test_nonfinite_input_is_rejected(void)
{
    const double input[] = {
        1.0,
        NAN,
        3.0
    };

    double output[3];

    assert(
        dct_ii_orthonormal(
            input,
            output,
            3U
        ) == DCT_NONFINITE_INPUT
    );
}

int main(void)
{
    test_constant_vector();
    test_known_vector();
    test_single_element_vector();
    test_energy_is_preserved();
    test_in_place_operation();
    test_invalid_arguments();
    test_nonfinite_input_is_rejected();

    printf("All orthonormal DCT-II tests passed.\n");

    return 0;
}
