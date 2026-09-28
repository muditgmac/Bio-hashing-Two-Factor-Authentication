#include "biohash.h"

#include "bbs.h"
#include "dct.h"
#include "gram_schmidt.h"

#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#define BIOHASH_RANDOM_BITS_PER_VALUE 16U
#define BIOHASH_MAX_BASIS_ATTEMPTS 8U

static double next_projection_value(BBSGenerator *generator)
{
    uint32_t value = 0U;

    for (
        size_t bit = 0U;
        bit < BIOHASH_RANDOM_BITS_PER_VALUE;
        ++bit
    ) {
        value = (value << 1U) | (uint32_t)bbs_next_bit(generator);
    }

    const double maximum =
        (double)((1U << BIOHASH_RANDOM_BITS_PER_VALUE) - 1U);

    return (2.0 * ((double)value / maximum)) - 1.0;
}

static int generate_orthonormal_basis(
    BBSGenerator *generator,
    double *raw_basis,
    double *orthonormal_basis,
    size_t rows,
    size_t cols,
    double tolerance
)
{
    const size_t element_count = rows * cols;

    for (
        size_t attempt = 0U;
        attempt < BIOHASH_MAX_BASIS_ATTEMPTS;
        ++attempt
    ) {
        for (size_t index = 0U; index < element_count; ++index) {
            raw_basis[index] = next_projection_value(generator);
        }

        const GramSchmidtStatus status =
            gram_schmidt_orthonormalize(
                raw_basis,
                orthonormal_basis,
                rows,
                cols,
                tolerance
            );

        if (status == GRAM_SCHMIDT_OK) {
            return 1;
        }
    }

    return 0;
}

BioHashStatus biohash_generate(
    const double *features,
    size_t feature_count,
    size_t hash_length,
    const BioHashConfig *config,
    uint8_t *output
)
{
    if (
        features == NULL ||
        config == NULL ||
        output == NULL ||
        feature_count == 0U ||
        hash_length == 0U ||
        hash_length > feature_count ||
        !isfinite(config->threshold) ||
        !isfinite(config->orthogonality_tolerance) ||
        config->orthogonality_tolerance < 0.0
    ) {
        return BIOHASH_INVALID_ARGUMENT;
    }

    for (size_t index = 0U; index < feature_count; ++index) {
        if (!isfinite(features[index])) {
            return BIOHASH_INVALID_ARGUMENT;
        }
    }

    BBSGenerator generator;

    if (!bbs_init(
            &generator,
            config->p,
            config->q,
            config->seed
        )) {
        return BIOHASH_BBS_FAILURE;
    }

    double *transformed_features =
        malloc(feature_count * sizeof(*transformed_features));

    double *raw_basis =
        malloc(
            feature_count *
            hash_length *
            sizeof(*raw_basis)
        );

    double *orthonormal_basis =
        malloc(
            feature_count *
            hash_length *
            sizeof(*orthonormal_basis)
        );

    if (
        transformed_features == NULL ||
        raw_basis == NULL ||
        orthonormal_basis == NULL
    ) {
        free(transformed_features);
        free(raw_basis);
        free(orthonormal_basis);

        return BIOHASH_ALLOCATION_FAILURE;
    }

    const DCTStatus dct_status =
        dct_ii_orthonormal(
            features,
            transformed_features,
            feature_count
        );

    if (dct_status != DCT_OK) {
        free(transformed_features);
        free(raw_basis);
        free(orthonormal_basis);

        return BIOHASH_DCT_FAILURE;
    }

    if (!generate_orthonormal_basis(
            &generator,
            raw_basis,
            orthonormal_basis,
            feature_count,
            hash_length,
            config->orthogonality_tolerance
        )) {
        free(transformed_features);
        free(raw_basis);
        free(orthonormal_basis);

        return BIOHASH_PROJECTION_FAILURE;
    }

    for (size_t column = 0U; column < hash_length; ++column) {
        double projection = 0.0;

        for (size_t row = 0U; row < feature_count; ++row) {
            projection +=
                transformed_features[row] *
                orthonormal_basis[
                    (row * hash_length) + column
                ];
        }

        output[column] =
            projection >= config->threshold ? 1U : 0U;
    }

    free(transformed_features);
    free(raw_basis);
    free(orthonormal_basis);

    return BIOHASH_OK;
}
