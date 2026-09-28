#include "gram_schmidt.h"

#include <math.h>
#include <stdint.h>

static size_t matrix_index(
    size_t row,
    size_t column,
    size_t column_count
)
{
    return (row * column_count) + column;
}

static double column_dot_product(
    const double *matrix,
    size_t rows,
    size_t cols,
    size_t first_column,
    size_t second_column
)
{
    double sum = 0.0;

    for (size_t row = 0U; row < rows; ++row) {
        const double first =
            matrix[matrix_index(row, first_column, cols)];
        const double second =
            matrix[matrix_index(row, second_column, cols)];

        sum += first * second;
    }

    return sum;
}

static double column_norm(
    const double *matrix,
    size_t rows,
    size_t cols,
    size_t column
)
{
    double squared_norm = 0.0;

    for (size_t row = 0U; row < rows; ++row) {
        const double value =
            matrix[matrix_index(row, column, cols)];

        squared_norm += value * value;
    }

    return sqrt(squared_norm);
}

GramSchmidtStatus gram_schmidt_orthonormalize(
    const double *input,
    double *output,
    size_t rows,
    size_t cols,
    double tolerance
)
{
    if (input == NULL || output == NULL) {
        return GRAM_SCHMIDT_INVALID_ARGUMENT;
    }

    if (rows == 0U || cols == 0U || cols > rows) {
        return GRAM_SCHMIDT_INVALID_DIMENSIONS;
    }

    if (rows > (SIZE_MAX / cols)) {
        return GRAM_SCHMIDT_INVALID_DIMENSIONS;
    }

    if (!(tolerance > 0.0) || !isfinite(tolerance)) {
        return GRAM_SCHMIDT_INVALID_ARGUMENT;
    }

    const size_t element_count = rows * cols;

    for (size_t index = 0U; index < element_count; ++index) {
        if (!isfinite(input[index])) {
            return GRAM_SCHMIDT_INVALID_ARGUMENT;
        }

        output[index] = input[index];
    }

    for (size_t column = 0U; column < cols; ++column) {
        /*
         * Modified Gram-Schmidt:
         *
         * Immediately update the current vector after removing
         * each projection instead of accumulating all projections
         * from the original vector.
         */
        for (
            size_t previous_column = 0U;
            previous_column < column;
            ++previous_column
        ) {
            const double projection =
                column_dot_product(
                    output,
                    rows,
                    cols,
                    previous_column,
                    column
                );

            for (size_t row = 0U; row < rows; ++row) {
                const size_t current_index =
                    matrix_index(row, column, cols);
                const size_t previous_index =
                    matrix_index(row, previous_column, cols);

                output[current_index] -=
                    projection * output[previous_index];
            }
        }

        const double norm =
            column_norm(output, rows, cols, column);

        if (!isfinite(norm) || norm <= tolerance) {
            return GRAM_SCHMIDT_LINEARLY_DEPENDENT;
        }

        for (size_t row = 0U; row < rows; ++row) {
            const size_t index =
                matrix_index(row, column, cols);

            output[index] /= norm;
        }
    }

    return GRAM_SCHMIDT_OK;
}
