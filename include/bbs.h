#ifndef BBS_H
#define BBS_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint64_t p;
    uint64_t q;
    uint64_t modulus;
    uint64_t state;
} BBSGenerator;

int bbs_is_prime(uint64_t value);
int bbs_is_blum_prime(uint64_t value);
uint64_t bbs_gcd(uint64_t a, uint64_t b);

int bbs_init(
    BBSGenerator *generator,
    uint64_t p,
    uint64_t q,
    uint64_t seed
);

uint64_t bbs_next_state(BBSGenerator *generator);
uint8_t bbs_next_bit(BBSGenerator *generator);

int bbs_generate_bits(
    BBSGenerator *generator,
    uint8_t *output,
    size_t count
);

#endif
