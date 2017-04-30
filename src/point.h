#ifndef INCLUDE_POINT_H_
#define INCLUDE_POINT_H_

#include "src/field.h"

struct curve25519_point_s {
  curve25519_num_t x;
  curve25519_num_t z;
  unsigned int normalized : 1;
};

typedef struct curve25519_point_s curve25519_point_t;

void curve25519_point_from_bin(curve25519_point_t* p, const uint8_t bin[32]);
void curve25519_point_zero(curve25519_point_t* p);

/* NOTE: `out` can be the same as `p` */
void curve25519_point_dbl(curve25519_point_t* out,
                          const curve25519_point_t* p);

/* NOTE: `out` can be the same as any of arguments */
void curve25519_point_diff_add(curve25519_point_t* out,
                               const curve25519_point_t* p1,
                               const curve25519_point_t* p2,
                               const curve25519_point_t* diff);

void curve25519_point_normalize(curve25519_point_t* out);

/* For tests */
void curve25519_point_init_ex(curve25519_point_t* p, const uint8_t x[32],
                              const uint8_t z[32]);

#endif  /* INCLUDE_POINT_H_ */
