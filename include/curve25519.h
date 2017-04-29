#ifndef INCLUDE_CURVE25519_H_
#define INCLUDE_CURVE25519_H_

#include <stdint.h>

/* 4 64-bit limbs */
struct curve25519_num_s {
  uint64_t limbs[4];
};

typedef struct curve25519_num_s curve25519_num_t;

void curve25519_num_add(curve25519_num_t* out, const curve25519_num_t* num);
void curve25519_num_sub(curve25519_num_t* out, const curve25519_num_t* num);
void curve25519_num_mul(curve25519_num_t* out,
                        const curve25519_num_t* a,
                        const curve25519_num_t* b);

/* Non constant-time methods */
void curve25519_num_normalize(curve25519_num_t* out);
int curve25519_num_cmp(const curve25519_num_t* a, const curve25519_num_t* b);

/* Helpers */

void curve25519_num_to_bin(uint8_t out[32], curve25519_num_t* num);
void curve25519_num_from_bin(curve25519_num_t* out, uint8_t bin[32]);

#endif  /* INCLUDE_CURVE25519_H_ */
