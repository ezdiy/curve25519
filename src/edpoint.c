#include <string.h>

#include "src/edpoint.h"
#include "src/field.h"
#include "src/common.h"


static int curve25519_ed__unpack(curve25519_num_t* out,
                                 const curve25519_num_t* num,
                                 const curve25519_num_t* denom);


static const curve25519_num_t kCurveD = {
  .limbs = {
    0xa3785913ca4deb75,
    0xabd841414d0a7000,
    0x98e879777940c78c,
    0x73fe6f2bee6c0352
  }
};


static const curve25519_num_t kCurvePm5d8 = {
  .limbs = {
    0xfdffffffffffffff,
    0xffffffffffffffff,
    0xffffffffffffffff,
    0xffffffffffffff0f
  }
};


int curve25519_ed_point_from_bin(curve25519_ed_point_t* p,
                                 const uint8_t bin[32]) {
  uint8_t copy[32];
  unsigned int is_odd;
  curve25519_num_t y2;
  curve25519_num_t num;
  curve25519_num_t denom;

  memcpy(copy, bin, sizeof(copy));
  is_odd = copy[sizeof(copy) - 1] & 0x80;
  copy[sizeof(copy) - 1] &= ~0x80;

  curve25519_num_from_bin(&p->y, copy);
  curve25519_num_one(&p->z);
  curve25519_num_zero(&p->t);
  p->normalized = 0;

  /* x^2 = (y^2 - 1) / (d y^2 + 1) */

  /* num = (y^2 - 1) */
  curve25519_num_sqr(&y2, &p->y);
  curve25519_num_sub(&num, &y2, &p->z);

  /* denom = (d y^2 + 1) */
  curve25519_num_mul(&denom, &y2, &kCurveD);
  curve25519_num_add(&denom, &denom, &p->z);

  if (curve25519_num_is_zero(&num)) {
    if (is_odd)
      return -1;

    curve25519_num_zero(&p->x);
    return 0;
  }

  if (0 != curve25519_ed__unpack(&p->x, &num, &denom))
    return -1;

  is_odd ^= curve25519_num_is_odd(&p->x);
  if (is_odd)
    curve25519_num_neg(&p->x);

  return 0;
}


int curve25519_ed__unpack(curve25519_num_t* out, const curve25519_num_t* num,
                          const curve25519_num_t* denom) {
  /* From: https://ed25519.cr.yp.to/ed25519-20110926.pdf
   *
   * beta = uv^3 (uv^7)^((q - 5) / 8)
   * alpha = +/- beta^2
   */

  curve25519_num_t uv7;
  curve25519_num_t pow;

  curve25519_num_sqr(out, denom);
  curve25519_num_mul(out, out, denom);
  curve25519_num_mul(out, out, num);

  curve25519_num_sqr(&uv7, denom);
  curve25519_num_sqr(&uv7, &uv7);
  curve25519_num_mul(&uv7, &uv7, out);


  /* Behold, very naive exponentiation. Result = out */
  curve25519_num_copy(&pow, &kCurvePm5d8);

  /* TODO(indutny): `is_zero` is slow */
  for (; !curve25519_num_is_zero(&pow); curve25519_num_sqr(&uv7, &uv7)) {
    if (curve25519_num_is_odd(&pow))
      curve25519_num_mul(out, out, &uv7);
    curve25519_num_shr(&pow, 1);
  }

  /* beta2 = pow */
  curve25519_num_sqr(&pow, out);

  /* beta2 * u = +/- v */
  curve25519_num_mul(&pow, &pow, num);

  if (curve25519_num_cmp(&pow, denom) == 0)
    return 0;

  curve25519_num_neg(out);
  curve25519_num_neg(&pow);
  if (curve25519_num_cmp(&pow, denom) != 0)
    return -1;

  return 0;
}


void curve25519_ed_point_dbl(curve25519_ed_point_t* out,
                             const curve25519_ed_point_t* p) {
}


void curve25519_ed_point_to_bin(uint8_t bin[32],
                                curve25519_ed_point_t* p) {
  curve25519_ed_point_normalize(p);

  curve25519_num_to_bin(bin, &p->y);
  if (curve25519_num_is_odd(&p->x))
    bin[31] |= 0x80;
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
