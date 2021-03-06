#include <string.h>

#include "src/edpoint.h"
#include "src/field.h"
#include "src/common.h"


static int curve25519_ed__unpack(curve25519_num_t* out,
                                 curve25519_num_t* num,
                                 curve25519_num_t* denom);


static const unsigned int kFieldSize = 255;


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


static const curve25519_num_t kCurveBaseX = {
  .limbs = {
    0x1ad5258f602d56c9,
    0xb2a7259560c72c69,
    0x5cdcd6fd31e2a4c0,
    0xfe536ecdd3366921
  }
};


static const curve25519_num_t kCurveBaseY = {
  .limbs = {
    0x5866666666666666,
    0x6666666666666666,
    0x6666666666666666,
    0x6666666666666666
  }
};


static const curve25519_num_t kCurveBaseT = {
  .limbs = {
    0xa3ddb7a5b38ade6d,
    0xf5525177809ff020,
    0x7de3ab648e4eea66,
    0x65768bd70f5f8767
  }
};


static const curve25519_num_t kTwo = {
  .limbs = { 2, 0, 0, 0 }
};


int curve25519_ed_point_from_bin(curve25519_ed_point_t* p,
                                 const uint8_t bin[32]) {
  uint8_t copy[32];
  unsigned int is_odd;
  curve25519_num_t y2;
  curve25519_num_t num;
  curve25519_num_t denom;

  memcpy(copy, bin, sizeof(copy));
  is_odd = (copy[sizeof(copy) - 1] & 0x80) ? 1 : 0;
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

  if (0 != curve25519_ed__unpack(&p->x, &num, &denom))
    return -1;

  is_odd ^= curve25519_num_is_odd(&p->x);
  if (is_odd)
    curve25519_num_neg(&p->x);

  curve25519_num_mul(&p->t, &p->x, &p->y);

  return 0;
}


int curve25519_ed__unpack(curve25519_num_t* out, curve25519_num_t* num,
                          curve25519_num_t* denom) {
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
  if (curve25519_num_ncmp(&pow, num) == 0)
    return 0;

  curve25519_num_mul(out, out, &kCurveRootM1);
  curve25519_num_neg(&pow);

  if (curve25519_num_ncmp(&pow, num) != 0)
    return -1;

  return 0;
}


void curve25519_ed_point_dbl(curve25519_ed_point_t* out,
                             curve25519_ed_point_t* p) {
  if (curve25519_ed_point_is_zero(p))
    return curve25519_ed_point_copy(out, p);

  curve25519_num_t a;
  curve25519_num_t b;
  curve25519_num_t d;
  curve25519_num_t e;
  curve25519_num_t g;
  curve25519_num_t h;

  curve25519_num_sqr(&a, &p->x);
  curve25519_num_sqr(&b, &p->y);
  curve25519_num_copy(&d, &a);
  curve25519_num_neg(&d);

  curve25519_num_add(&e, &p->x, &p->y);
  curve25519_num_sqr(&e, &e);
  curve25519_num_sub(&e, &e, &a);
  curve25519_num_sub(&e, &e, &b);
  curve25519_num_add(&g, &d, &b);
  curve25519_num_sub(&h, &d, &b);

  if (p->normalized) {
    /* http://hyperelliptic.org/EFD/g1p/auto-twisted-extended-1.html#doubling-mdbl-2008-hwcd */

    curve25519_num_sub(&out->x, &g, &kTwo);
    curve25519_num_mul(&out->x, &out->x, &e);
    curve25519_num_mul(&out->y, &g, &h);
    curve25519_num_mul(&out->t, &e, &h);

    curve25519_num_mul(&out->z, &g, &kTwo);
    curve25519_num_sqr(&a, &g);
    curve25519_num_sub(&out->z, &a, &out->z);
  } else {
    /* http://hyperelliptic.org/EFD/g1p/auto-twisted-extended-1.html#doubling-dbl-2008-hwcd */
    curve25519_num_t c;
    curve25519_num_t f;

    curve25519_num_sqr(&c, &p->z);
    curve25519_num_mul(&c, &c, &kTwo);
    curve25519_num_sub(&f, &g, &c);

    curve25519_num_mul(&out->x, &e, &f);
    curve25519_num_mul(&out->y, &g, &h);
    curve25519_num_mul(&out->t, &e, &h);
    curve25519_num_mul(&out->z, &f, &g);
  }

  out->normalized = 0;
}


void curve25519_ed_point_add(curve25519_ed_point_t* out,
                             curve25519_ed_point_t* p1,
                             curve25519_ed_point_t* p2) {
  if (curve25519_ed_point_is_zero(p1))
    return curve25519_ed_point_copy(out, p2);
  if (curve25519_ed_point_is_zero(p2))
    return curve25519_ed_point_copy(out, p1);

  curve25519_num_t a;
  curve25519_num_t b;
  curve25519_num_t c;
  curve25519_num_t d;
  curve25519_num_t e;
  curve25519_num_t f;
  curve25519_num_t g;
  curve25519_num_t h;

  if (p1->normalized && !p2->normalized) {
    /* Re-order to get to `p2->normalized` */
    curve25519_ed_point_add(out, p2, p1);
    return;
  }

  curve25519_num_add(&a, &p2->y, &p2->x);
  curve25519_num_sub(&c, &p1->y, &p1->x);
  curve25519_num_mul(&a, &a, &c);

  curve25519_num_sub(&b, &p2->y, &p2->x);
  curve25519_num_add(&c, &p1->y, &p1->x);
  curve25519_num_mul(&b, &b, &c);

  curve25519_num_mul(&c, &p2->t, &kTwo);
  curve25519_num_mul(&d, &p1->t, &kTwo);

  if (p1->normalized && p2->normalized) {
    /* http://hyperelliptic.org/EFD/g1p/auto-twisted-extended-1.html#addition-mmadd-2008-hwcd-4 */
  } else if (p2->normalized) {
    /* http://hyperelliptic.org/EFD/g1p/auto-twisted-extended-1.html#addition-madd-2008-hwcd-4 */
    curve25519_num_mul(&c, &c, &p1->z);
  } else {
    curve25519_num_mul(&c, &c, &p1->z);
    curve25519_num_mul(&d, &d, &p2->z);
  }

  curve25519_num_add(&e, &d, &c);
  curve25519_num_sub(&f, &b, &a);
  curve25519_num_add(&g, &b, &a);
  curve25519_num_sub(&h, &d, &c);

  curve25519_num_mul(&out->x, &e, &f);
  curve25519_num_mul(&out->y, &g, &h);
  curve25519_num_mul(&out->t, &e, &h);
  curve25519_num_mul(&out->z, &f, &g);

  out->normalized = 0;
}


void curve25519_ed_point_scalar_mul(curve25519_ed_point_t* out,
                                    const curve25519_ed_point_t* p,
                                    const uint8_t scalar[32]) {
  curve25519_ed_point_t hole;
  curve25519_ed_point_t runner;
  unsigned int i;

  curve25519_ed_point_zero(&hole);
  curve25519_ed_point_copy(&runner, p);
  curve25519_ed_point_zero(out);

  for (i = 0; i < kFieldSize; i++) {
    uint8_t bit;

    bit = scalar[i / 8] & (1 << (i % 8));
    if (bit)
      curve25519_ed_point_add(out, out, &runner);
    else
      curve25519_ed_point_add(&hole, &hole, &runner);
    curve25519_ed_point_dbl(&runner, &runner);
  }
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


void curve25519_ed_point_base(curve25519_ed_point_t* out) {
  curve25519_num_copy(&out->x, &kCurveBaseX);
  curve25519_num_copy(&out->y, &kCurveBaseY);
  curve25519_num_one(&out->z);
  curve25519_num_copy(&out->t, &kCurveBaseT);
  out->normalized = 1;
}


void curve25519_ed_point_zero(curve25519_ed_point_t* out) {
  curve25519_num_zero(&out->x);
  curve25519_num_one(&out->y);
  curve25519_num_one(&out->z);
  curve25519_num_zero(&out->t);
  out->normalized = 1;
}


int curve25519_ed_point_is_zero(curve25519_ed_point_t* num) {
  return curve25519_num_is_zero(&num->x) &&
         curve25519_num_ncmp(&num->y, &num->z) == 0;
}


void curve25519_ed_point_copy(curve25519_ed_point_t* out,
                              const curve25519_ed_point_t* p) {
  curve25519_num_copy(&out->x, &p->x);
  curve25519_num_copy(&out->y, &p->y);
  curve25519_num_copy(&out->z, &p->z);
  curve25519_num_copy(&out->t, &p->t);
  out->normalized = p->normalized;
}


void curve25519_ed_point_init_ex(curve25519_ed_point_t* p, const uint8_t x[32],
                                 const uint8_t y[32], const uint8_t z[32],
                                 const uint8_t t[32]) {
  curve25519_num_from_bin(&p->x, x);
  curve25519_num_from_bin(&p->y, y);
  curve25519_num_from_bin(&p->z, z);
  curve25519_num_from_bin(&p->t, t);
  p->normalized = curve25519_num_is_one(&p->z);
}
