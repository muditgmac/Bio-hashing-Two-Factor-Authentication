#include "bbs.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

static void test_gcd(void)
{
    assert(bbs_gcd(48U, 18U) == 6U);
    assert(bbs_gcd(17U, 13U) == 1U);
    assert(bbs_gcd(0U, 7U) == 7U);
}

static void test_prime_detection(void)
{
    assert(bbs_is_prime(2U));
    assert(bbs_is_prime(3U));
    assert(bbs_is_prime(383U));
    assert(bbs_is_prime(503U));

    assert(!bbs_is_prime(0U));
    assert(!bbs_is_prime(1U));
    assert(!bbs_is_prime(4U));
    assert(!bbs_is_prime(21U));
}

static void test_blum_prime_detection(void)
{
    assert(bbs_is_blum_prime(383U));
    assert(bbs_is_blum_prime(503U));

    assert(!bbs_is_blum_prime(2U));
    assert(!bbs_is_blum_prime(5U));
    assert(!bbs_is_blum_prime(13U));
}

static void test_valid_initialization(void)
{
    BBSGenerator generator;

    assert(bbs_init(&generator, 383U, 503U, 271U));

    assert(generator.p == 383U);
    assert(generator.q == 503U);
    assert(generator.modulus == 192649U);
    assert(generator.state > 0U);
    assert(generator.state < generator.modulus);
}

static void test_invalid_parameters(void)
{
    BBSGenerator generator;

    assert(!bbs_init(NULL, 383U, 503U, 271U));
    assert(!bbs_init(&generator, 5U, 503U, 271U));
    assert(!bbs_init(&generator, 383U, 13U, 271U));
    assert(!bbs_init(&generator, 383U, 383U, 271U));
    assert(!bbs_init(&generator, 383U, 503U, 1U));
    assert(!bbs_init(&generator, 383U, 503U, 383U));
}

static void test_state_recurrence(void)
{
    BBSGenerator generator;

    assert(bbs_init(&generator, 383U, 503U, 271U));

    uint64_t previous = generator.state;
    uint64_t next = bbs_next_state(&generator);

    assert(next < generator.modulus);
    assert(next != 0U);
    assert(previous != 0U);
}

static void test_bit_generation_is_deterministic(void)
{
    BBSGenerator first;
    BBSGenerator second;

    uint8_t first_bits[64];
    uint8_t second_bits[64];

    assert(bbs_init(&first, 383U, 503U, 271U));
    assert(bbs_init(&second, 383U, 503U, 271U));

    assert(bbs_generate_bits(&first, first_bits, 64U));
    assert(bbs_generate_bits(&second, second_bits, 64U));

    for (size_t index = 0U; index < 64U; ++index) {
        assert(first_bits[index] == second_bits[index]);
        assert(first_bits[index] == 0U || first_bits[index] == 1U);
    }
}

static void test_different_seed_changes_stream(void)
{
    BBSGenerator first;
    BBSGenerator second;

    uint8_t first_bits[64];
    uint8_t second_bits[64];

    assert(bbs_init(&first, 383U, 503U, 271U));
    assert(bbs_init(&second, 383U, 503U, 269U));

    assert(bbs_generate_bits(&first, first_bits, 64U));
    assert(bbs_generate_bits(&second, second_bits, 64U));

    int difference_found = 0;

    for (size_t index = 0U; index < 64U; ++index) {
        if (first_bits[index] != second_bits[index]) {
            difference_found = 1;
            break;
        }
    }

    assert(difference_found);
}

int main(void)
{
    test_gcd();
    test_prime_detection();
    test_blum_prime_detection();
    test_valid_initialization();
    test_invalid_parameters();
    test_state_recurrence();
    test_bit_generation_is_deterministic();
    test_different_seed_changes_stream();

    printf("All BBS tests passed.\n");

    return 0;
}
