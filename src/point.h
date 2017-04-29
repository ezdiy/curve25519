#ifndef INCLUDE_POINT_H_
#define INCLUDE_POINT_H_

#include "src/field.h"

struct curve25519_point_s {
  curve25519_num_t x;
  curve25519_num_t z;
  unsigned int normalized : 1;
};

typedef struct curve25519_point_s curve25519_point_t;

void curve25519_point_init(curve25519_point_t* p, const uint8_t* x);
void curve25519_point_dbl(curve25519_point_t* p);

#endif  /* INCLUDE_POINT_H_ */
