#ifndef INCLUDE_CURVE25519_H_
#define INCLUDE_CURVE25519_H_

#include <stdint.h>

typedef struct curve25519_table_s curve25519_table_t;

struct curve25519_table_s {
  uint8_t points[32][256];
};

void curve25519_precompute(curve25519_table_t* out, uint8_t point[32]);

#endif  /* INCLUDE_CURVE25519_H_ */
