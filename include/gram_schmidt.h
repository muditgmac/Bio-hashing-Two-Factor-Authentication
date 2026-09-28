#ifndef GRAM_SCHMIDT_H
#define GRAM_SCHMIDT_H

#include <stddef.h>

typedef enum {
    GRAM_SCHMIDT_OK = 0,
    GRAM_SCHMIDT_INVALID_ARGUMENT,
    GRAM_SCHMIDT_INVALID_DIMENSIONS,
    GRAM_SCHMIDT_LINEARLY_DEPENDENT
} GramSchmidtStatus;

/*
 * Orthonormalize the columns of a row-major matrix using
 * Modified Gram-Schmidt.
 *
 * input:
 *     Row-major matrix with shape rows x cols.
 *
 * output:
 *     Row-major matrix receiving the orthonormalized columns.
 *     input and output may refer to the same matrix.
 *
 * tolerance:
 *     A column norm less than or equal to this value is treated
 *     as numerically linearly dependent.
 *
 * Returns:
 *     GRAM_SCHMIDT_OK on success.
 */
GramSchmidtStatus gram_schmidt_orthonormalize(
    const double *input,
    double *output,
    size_t rows,
    size_t cols,
    double tolerance
);

#endif
