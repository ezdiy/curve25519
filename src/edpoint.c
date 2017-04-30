#include <string.h>

#include "src/edpoint.h"
#include "src/field.h"
#include "src/common.h"


static const curve25519_num_t kCurveD = {
  .limbs = {
    0xa3785913ca4deb75,
    0xabd841414d0a7000,
    0x98e879777940c78c,
    0x73fe6f2bee6c0352
  }
};


int curve25519_ed_point_init(curve25519_ed_point_t* p, const uint8_t bin[32]) {
  uint8_t copy[32];
  unsigned int is_odd;
  curve25519_num_t x2;
  curve25519_num_t y2;
  curve25519_num_t lhs;
  curve25519_num_t rhs;

  memcpy(copy, bin, sizeof(copy));
  is_odd = copy[sizeof(copy) - 1] & 0x80;
  copy[sizeof(copy) - 1] &= ~0x80;

  curve25519_num_from_bin(&p->y, copy);
  curve25519_num_one(&p->z);
  curve25519_num_zero(&p->t);
  p->normalized = 0;

  /* x^2 = (y^2 - 1) / (d y^2 + 1) */
  curve25519_num_sqr(&y2, &p->y);
  curve25519_num_sub(&lhs, &y2, &p->z);
  curve25519_num_mul(&rhs, &y2, &kCurveD);
  curve25519_num_add(&rhs, &rhs, &p->z);
  curve25519_num_inv(&rhs, &rhs);
  curve25519_num_mul(&x2, &lhs, &rhs);

  if (curve25519_num_is_zero(&x2)) {
    if (is_odd)
      return -1;

    curve25519_num_zero(&p->x);
    return 0;
  }

  curve25519_num_sqrt(&p->x, &x2);
  curve25519_num_sqr(&lhs, &p->x);

  curve25519_num_normalize(&x2);
  curve25519_num_normalize(&lhs);
  if (curve25519_num_cmp(&x2, &lhs) != 0)
    return -1;

  is_odd ^= curve25519_num_is_odd(&p->x);
  if (is_odd)
    curve25519_num_sub(&p->x, &p->t, &p->x);

  return 0;
}


void curve25519_ed_point_dbl(curve25519_ed_point_t* out,
                             const curve25519_ed_point_t* p) {
}


void curve25519_ed_point_to_bin(uint8_t bin[32],
                                const curve25519_ed_point_t* p) {
}


void curve25519_ed_point_normalize(curve25519_ed_point_t* out) {
  curve25519_num_t zinv;

  if (out->normalized)
    return;

  curve25519_num_inv(&zinv, &out->z);
  curve25519_num_mul(&out->x, &out->x, &zinv);
  curve25519_num_mul(&out->y, &out->y, &zinv);
  curve25519_num_mul(&out->t, &out->t, &zinv);
  curve25519_num_one(&out->z);
  out->normalized = 1;
}


void curve25519_ed_point_init_ex(curve25519_ed_point_t* p, const uint8_t x[32],
                                 const uint8_t y[32], const uint8_t z[32],
                                 const uint8_t t[32]) {
}
