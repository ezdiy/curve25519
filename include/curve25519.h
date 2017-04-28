#ifndef INCLUDE_CURVE25519_H_
#define INCLUDE_CURVE25519_H_

#include <stdint.h>

/* 4 64-bit limbs + 4 extra limbs */
struct curve25519_num_s {
  uint64_t limbs[8];
};

typedef struct curve25519_num_s curve25519_num_t;

void curve25519_num_normalize(curve25519_num_t* out);
void curve25519_num_add(curve25519_num_t* out, const curve25519_num_t* num);
void curve25519_num_sub(curve25519_num_t* out, const curve25519_num_t* num);

void curve25519_num_to_bin(uint8_t out[32], curve25519_num_t* num);
void curve25519_num_from_bin(curve25519_num_t* out, uint8_t bin[32]);

#endif  /* INCLUDE_CURVE25519_H_ */
