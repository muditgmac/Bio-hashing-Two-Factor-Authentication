#include "bbs.h"

#include <limits.h>

uint64_t bbs_gcd(uint64_t a, uint64_t b)
{
    while (b != 0U) {
        uint64_t remainder = a % b;
        a = b;
        b = remainder;
    }

    return a;
}

int bbs_is_prime(uint64_t value)
{
    if (value < 2U) {
        return 0;
    }

    if (value == 2U) {
        return 1;
    }

    if ((value % 2U) == 0U) {
        return 0;
    }

    for (uint64_t divisor = 3U;
         divisor <= value / divisor;
         divisor += 2U) {
        if ((value % divisor) == 0U) {
            return 0;
        }
    }

    return 1;
}

int bbs_is_blum_prime(uint64_t value)
{
    return bbs_is_prime(value) && ((value % 4U) == 3U);
}

static uint64_t modular_multiply(
    uint64_t a,
    uint64_t b,
    uint64_t modulus
)
{
    uint64_t result = 0U;

    a %= modulus;

    while (b != 0U) {
        if ((b & 1U) != 0U) {
            if (result >= modulus - a) {
                result = result - (modulus - a);
            } else {
                result += a;
            }
        }

        b >>= 1U;

        if (b != 0U) {
            if (a >= modulus - a) {
                a = a - (modulus - a);
            } else {
                a += a;
            }
        }
    }

    return result;
}

int bbs_init(
    BBSGenerator *generator,
    uint64_t p,
    uint64_t q,
    uint64_t seed
)
{
    if (generator == NULL) {
        return 0;
    }

    if (!bbs_is_blum_prime(p) || !bbs_is_blum_prime(q)) {
        return 0;
    }

    if (p == q) {
        return 0;
    }

    if (p > UINT64_MAX / q) {
        return 0;
    }

    uint64_t modulus = p * q;

    if (seed <= 1U || seed >= modulus) {
        return 0;
    }

    if (bbs_gcd(seed, modulus) != 1U) {
        return 0;
    }

    generator->p = p;
    generator->q = q;
    generator->modulus = modulus;
    generator->state = modular_multiply(seed, seed, modulus);

    return 1;
}

uint64_t bbs_next_state(BBSGenerator *generator)
{
    if (generator == NULL || generator->modulus == 0U) {
        return 0U;
    }

    generator->state = modular_multiply(
        generator->state,
        generator->state,
        generator->modulus
    );

    return generator->state;
}

uint8_t bbs_next_bit(BBSGenerator *generator)
{
    uint64_t state = bbs_next_state(generator);
    return (uint8_t)(state & 1U);
}

int bbs_generate_bits(
    BBSGenerator *generator,
    uint8_t *output,
    size_t count
)
{
    if (generator == NULL) {
        return 0;
    }

    if (count > 0U && output == NULL) {
        return 0;
    }

    for (size_t index = 0U; index < count; ++index) {
        output[index] = bbs_next_bit(generator);
    }

    return 1;
}
