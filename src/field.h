#ifndef INCLUDE_FIELD_H_
#define INCLUDE_FIELD_H_

#include <stdint.h>

/* 4 64-bit limbs */
struct curve25519_num_s {
  uint64_t limbs[4];
};

typedef struct curve25519_num_s curve25519_num_t;

/* NOTE: `out` can be the same as `a` */
void curve25519_num_add(curve25519_num_t* out, const curve25519_num_t* a,
                        const curve25519_num_t* b);
void curve25519_num_sub(curve25519_num_t* out, const curve25519_num_t* a,
                        const curve25519_num_t* b);
void curve25519_num_mul(curve25519_num_t* out, const curve25519_num_t* a,
                        const curve25519_num_t* b);

/* NOTE: `out` can be the same as `num` */
void curve25519_num_sqr(curve25519_num_t* out, const curve25519_num_t* num);
void curve25519_num_inv(curve25519_num_t* out, const curve25519_num_t* num);

void curve25519_num_shr(curve25519_num_t* num, uint8_t shift);

/* Non constant-time methods */
void curve25519_num_normalize(curve25519_num_t* out);

/* NOTE: assumes normalized values */
int curve25519_num_cmp(const curve25519_num_t* a, const curve25519_num_t* b);

/* Does not assume normalized values */
int curve25519_num_ncmp(curve25519_num_t* a, curve25519_num_t* b);

/* Helpers */

void curve25519_num_copy(curve25519_num_t* out, const curve25519_num_t* num);
void curve25519_num_to_bin(uint8_t out[32], curve25519_num_t* num);
void curve25519_num_from_bin(curve25519_num_t* out, const uint8_t bin[32]);
void curve25519_num_one(curve25519_num_t* out);
void curve25519_num_zero(curve25519_num_t* out);
void curve25519_num_neg(curve25519_num_t* num);

/* Don't use in performance-critical code */
int curve25519_num_is_one(curve25519_num_t* num);
int curve25519_num_is_zero(curve25519_num_t* num);
int curve25519_num_is_odd(curve25519_num_t* num);

#endif  /* INCLUDE_FIELD_H_ */
