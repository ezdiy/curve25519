#include <string.h>

#include "src/edpoint.h"
#include "src/field.h"
#include "src/common.h"


static int curve25519_ed__unpack(curve25519_num_t* out,
                                 const curve25519_num_t* num,
                                 const curve25519_num_t* denom);


static const curve25519_num_t kCurveD = {
  .limbs = {
    0x75eb4dca135978a3,
    0x00700a4d4141d8ab,
    0x8cc740797779e898,
    0x52036cee2b6ffe73
  }
};


static const curve25519_num_t kCurvePm5d8 = {
  .limbs = {
    0xfffffffffffffffd,
    0xffffffffffffffff,
    0xffffffffffffffff,
    0x0fffffffffffffff
  }
};


static const curve25519_num_t kCurveRootM1 = {
  .limbs = {
    0x3b11e4d8b5f15f3d,
    0xd0bce7f952d01b87,
    0xd4b2ff66c2042858,
    0x547cdb7fb03e20f4
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
  p->normalized = 1;

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
    curve25519_num_zero(&p->t);
    return 0;
  }

  /* TODO(indutny): these normalize calls should be in `field.c` */
  curve25519_num_normalize(&num);
  if (0 != curve25519_ed__unpack(&p->x, &num, &denom))
    return -1;

  curve25519_num_normalize(&p->x);
  is_odd ^= curve25519_num_is_odd(&p->x);
  if (is_odd)
    curve25519_num_neg(&p->x);

  curve25519_num_mul(&p->t, &p->x, &p->y);

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
  curve25519_num_mul(&pow, &pow, denom);

  curve25519_num_normalize(&pow);
  if (curve25519_num_cmp(&pow, num) == 0)
    return 0;

  curve25519_num_mul(out, out, &kCurveRootM1);
  curve25519_num_neg(&pow);

  curve25519_num_normalize(&pow);
  if (curve25519_num_cmp(&pow, num) != 0)
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
