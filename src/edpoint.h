#ifndef INCLUDE_EDPOINT_H_
#define INCLUDE_EDPOINT_H_

#include "src/field.h"

struct curve25519_ed_point_s {
  curve25519_num_t x;
  curve25519_num_t y;
  curve25519_num_t z;
  curve25519_num_t t;
  unsigned int normalized : 1;
};

typedef struct curve25519_ed_point_s curve25519_ed_point_t;

int curve25519_ed_point_init(curve25519_ed_point_t* p, const uint8_t bin[32]);

/* NOTE: `out` can be the same as `p` */
void curve25519_ed_point_dbl(curve25519_ed_point_t* out,
                             const curve25519_ed_point_t* p);

void curve25519_ed_point_to_bin(uint8_t bin[32],
                                const curve25519_ed_point_t* p);

void curve25519_ed_point_normalize(curve25519_ed_point_t* out);

/* For tests */
void curve25519_ed_point_init_ex(curve25519_ed_point_t* p, const uint8_t x[32],
                                 const uint8_t y[32], const uint8_t z[32],
                                 const uint8_t t[32]);

#endif  /* INCLUDE_EDPOINT_H_ */
