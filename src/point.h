#ifndef INCLUDE_POINT_H_
#define INCLUDE_POINT_H_

#include "src/field.h"

struct curve25519_point_s {
  curve25519_num_t x;
  curve25519_num_t z;
};

typedef struct curve25519_point_s curve25519_point_t;

#endif  /* INCLUDE_POINT_H_ */
