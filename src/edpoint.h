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

int curve25519_ed_point_from_bin(curve25519_ed_point_t* p,
                                 const uint8_t bin[32]);
void curve25519_ed_point_to_bin(uint8_t bin[32],
                                curve25519_ed_point_t* p);

/* NOTE: `out` can be the same as `p` */
void curve25519_ed_point_dbl(curve25519_ed_point_t* out,
                             curve25519_ed_point_t* p);

/* NOTE: any two points may be the same */
void curve25519_ed_point_add(curve25519_ed_point_t* out,
                             curve25519_ed_point_t* p1,
                             curve25519_ed_point_t* p2);

/* NOTE: `out` can be the same as `p` */
void curve25519_ed_point_scalar_mul(curve25519_ed_point_t* out,
                                    const curve25519_ed_point_t* p,
                                    const uint8_t scalar[32]);

void curve25519_ed_point_normalize(curve25519_ed_point_t* out);
void curve25519_ed_point_zero(curve25519_ed_point_t* out);
int curve25519_ed_point_is_zero(curve25519_ed_point_t* num);
void curve25519_ed_point_copy(curve25519_ed_point_t* out,
                              const curve25519_ed_point_t* p);

/* For tests */
void curve25519_ed_point_init_ex(curve25519_ed_point_t* p, const uint8_t x[32],
                                 const uint8_t y[32], const uint8_t z[32],
                                 const uint8_t t[32]);

#endif  /* INCLUDE_EDPOINT_H_ */
