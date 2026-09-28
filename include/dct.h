#ifndef DCT_H
#define DCT_H

#include <stddef.h>

typedef enum {
    DCT_OK = 0,
    DCT_INVALID_ARGUMENT,
    DCT_NONFINITE_INPUT,
    DCT_ALLOCATION_FAILURE
} DCTStatus;

/*
 * Compute an orthonormal DCT-II.
 *
 * input:
 *     Input vector of length 'length'.
 *
 * output:
 *     Output vector of the same length.
 *     input and output may point to the same memory.
 *
 * length:
 *     Number of elements. Must be greater than zero.
 *
 * Returns:
 *     DCT_OK on success.
 */
DCTStatus dct_ii_orthonormal(
    const double *input,
    double *output,
    size_t length
);

#endif
