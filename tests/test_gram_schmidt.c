#include "gram_schmidt.h"

#include <assert.h>
#include <math.h>
#include <stdio.h>

#define TEST_TOLERANCE 1e-12
#define CHECK_TOLERANCE 1e-10

static size_t matrix_index(
    size_t row,
    size_t column,
    size_t cols
)
{
    return (row * cols) + column;
}

static double column_dot(
    const double *matrix,
    size_t rows,
    size_t cols,
    size_t first_column,
    size_t second_column
)
{
    double result = 0.0;

    for (size_t row = 0U; row < rows; ++row) {
        result +=
            matrix[matrix_index(row, first_column, cols)] *
            matrix[matrix_index(row, second_column, cols)];
    }

    return result;
}

static double column_norm(
    const double *matrix,
    size_t rows,
    size_t cols,
    size_t column
)
{
    return sqrt(
        column_dot(
            matrix,
            rows,
            cols,
            column,
            column
        )
    );
}

static void test_identity_matrix_remains_orthonormal(void)
{
    const double input[] = {
        1.0, 0.0, 0.0,
        0.0, 1.0, 0.0,
        0.0, 0.0, 1.0
    };

    double output[9];

    const GramSchmidtStatus status =
        gram_schmidt_orthonormalize(
            input,
            output,
            3U,
            3U,
            TEST_TOLERANCE
        );

    assert(status == GRAM_SCHMIDT_OK);

    for (size_t index = 0U; index < 9U; ++index) {
        assert(fabs(output[index] - input[index]) < CHECK_TOLERANCE);
    }
}

static void test_columns_become_orthonormal(void)
{
    /*
     * Columns:
     *
     * c0 = [1, 1, 0]^T
     * c1 = [1, 0, 1]^T
     */
    const double input[] = {
        1.0, 1.0,
        1.0, 0.0,
        0.0, 1.0
    };

    double output[6];

    const GramSchmidtStatus status =
        gram_schmidt_orthonormalize(
            input,
            output,
            3U,
            2U,
            TEST_TOLERANCE
        );

    assert(status == GRAM_SCHMIDT_OK);

    assert(
        fabs(column_norm(output, 3U, 2U, 0U) - 1.0) <
        CHECK_TOLERANCE
    );

    assert(
        fabs(column_norm(output, 3U, 2U, 1U) - 1.0) <
        CHECK_TOLERANCE
    );

    assert(
        fabs(column_dot(output, 3U, 2U, 0U, 1U)) <
        CHECK_TOLERANCE
    );
}

static void test_linearly_dependent_columns_are_rejected(void)
{
    /*
     * c1 = 2 * c0
     */
    const double input[] = {
        1.0, 2.0,
        2.0, 4.0,
        3.0, 6.0
    };

    double output[6];

    const GramSchmidtStatus status =
        gram_schmidt_orthonormalize(
            input,
            output,
            3U,
            2U,
            TEST_TOLERANCE
        );

    assert(status == GRAM_SCHMIDT_LINEARLY_DEPENDENT);
}

static void test_invalid_dimensions_are_rejected(void)
{
    const double input[] = {
        1.0, 0.0,
        0.0, 1.0
    };

    double output[4];

    assert(
        gram_schmidt_orthonormalize(
            input,
            output,
            0U,
            2U,
            TEST_TOLERANCE
        ) == GRAM_SCHMIDT_INVALID_DIMENSIONS
    );

    assert(
        gram_schmidt_orthonormalize(
            input,
            output,
            2U,
            3U,
            TEST_TOLERANCE
        ) == GRAM_SCHMIDT_INVALID_DIMENSIONS
    );
}

static void test_invalid_arguments_are_rejected(void)
{
    const double input[] = {
        1.0, 0.0,
        0.0, 1.0
    };

    double output[4];

    assert(
        gram_schmidt_orthonormalize(
            NULL,
            output,
            2U,
            2U,
            TEST_TOLERANCE
        ) == GRAM_SCHMIDT_INVALID_ARGUMENT
    );

    assert(
        gram_schmidt_orthonormalize(
            input,
            NULL,
            2U,
            2U,
            TEST_TOLERANCE
        ) == GRAM_SCHMIDT_INVALID_ARGUMENT
    );

    assert(
        gram_schmidt_orthonormalize(
            input,
            output,
            2U,
            2U,
            0.0
        ) == GRAM_SCHMIDT_INVALID_ARGUMENT
    );
}

static void test_nonfinite_input_is_rejected(void)
{
    const double input[] = {
        1.0, 0.0,
        NAN, 1.0
    };

    double output[4];

    const GramSchmidtStatus status =
        gram_schmidt_orthonormalize(
            input,
            output,
            2U,
            2U,
            TEST_TOLERANCE
        );

    assert(status == GRAM_SCHMIDT_INVALID_ARGUMENT);
}

static void test_in_place_operation(void)
{
    double matrix[] = {
        1.0, 1.0,
        1.0, 0.0,
        0.0, 1.0
    };

    const GramSchmidtStatus status =
        gram_schmidt_orthonormalize(
            matrix,
            matrix,
            3U,
            2U,
            TEST_TOLERANCE
        );

    assert(status == GRAM_SCHMIDT_OK);

    assert(
        fabs(column_norm(matrix, 3U, 2U, 0U) - 1.0) <
        CHECK_TOLERANCE
    );

    assert(
        fabs(column_norm(matrix, 3U, 2U, 1U) - 1.0) <
        CHECK_TOLERANCE
    );

    assert(
        fabs(column_dot(matrix, 3U, 2U, 0U, 1U)) <
        CHECK_TOLERANCE
    );
}

int main(void)
{
    test_identity_matrix_remains_orthonormal();
    test_columns_become_orthonormal();
    test_linearly_dependent_columns_are_rejected();
    test_invalid_dimensions_are_rejected();
    test_invalid_arguments_are_rejected();
    test_nonfinite_input_is_rejected();
    test_in_place_operation();

    printf("All Modified Gram-Schmidt tests passed.\n");

    return 0;
}
