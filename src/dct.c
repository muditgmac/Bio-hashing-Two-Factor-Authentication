#include "dct.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

static int vector_is_finite(
    const double *values,
    size_t length
)
{
    for (size_t index = 0U; index < length; ++index) {
        if (!isfinite(values[index])) {
            return 0;
        }
    }

    return 1;
}

DCTStatus dct_ii_orthonormal(
    const double *input,
    double *output,
    size_t length
)
{
    if (input == NULL || output == NULL || length == 0U) {
        return DCT_INVALID_ARGUMENT;
    }

    if (!vector_is_finite(input, length)) {
        return DCT_NONFINITE_INPUT;
    }

    const double *source = input;
    double *scratch = NULL;

    if (input == output) {
        scratch = malloc(length * sizeof(*scratch));

        if (scratch == NULL) {
            return DCT_ALLOCATION_FAILURE;
        }

        memcpy(
            scratch,
            input,
            length * sizeof(*scratch)
        );

        source = scratch;
    }

    const double n = (double)length;
    const double pi = acos(-1.0);

    for (size_t frequency = 0U;
         frequency < length;
         ++frequency) {

        double sum = 0.0;

        const double frequency_value =
            (double)frequency;

        for (size_t sample = 0U;
             sample < length;
             ++sample) {

            const double sample_term =
                (2.0 * (double)sample) + 1.0;

            const double angle =
                (
                    pi *
                    sample_term *
                    frequency_value
                ) /
                (2.0 * n);

            sum += source[sample] * cos(angle);
        }

        const double scale =
            frequency == 0U
                ? sqrt(1.0 / n)
                : sqrt(2.0 / n);

        output[frequency] = scale * sum;
    }

    free(scratch);

    return DCT_OK;
}
