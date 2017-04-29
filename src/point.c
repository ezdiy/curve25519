#include "src/point.h"
#include "src/field.h"
#include "src/common.h"


static const curve25519_num_t kCurveA = {
  .limbs = { 0x76d06, 0, 0, 0 }
};


static const curve25519_num_t kCurveA24 = {
  .limbs = { 0x1db42, 0, 0, 0 }
};


static const curve25519_num_t kFour = {
  .limbs = { 4, 0, 0, 0 }
};


void curve25519_point_init(curve25519_point_t* p, const uint8_t x[32]) {
  curve25519_num_from_bin(&p->x, x);
  curve25519_num_one(&p->z);
  p->normalized = 1;
}


void curve25519_point_zero(curve25519_point_t* p) {
  curve25519_num_one(&p->x);
  curve25519_num_zero(&p->z);
  p->normalized = 0;
}


/* For tests */
void curve25519_point_init_ex(curve25519_point_t* p, const uint8_t x[32],
                              const uint8_t z[32]) {
  curve25519_num_from_bin(&p->x, x);
  curve25519_num_from_bin(&p->z, z);
  p->normalized = curve25519_num_is_one(&p->z);
}


/* TODO(indutny): rewrite for fast squaring */
void curve25519_point_dbl(curve25519_point_t* out,
                          const curve25519_point_t* p) {
  if (p->normalized) {
    /* http://hyperelliptic.org/EFD/g1p/auto-montgom-xz.html#doubling-mdbl-1987-m */
    curve25519_num_t xx1;
    curve25519_num_t x3;
    curve25519_num_t z3;

    /* xx1 = x^2 */
    curve25519_num_sqr(&xx1, &p->x);

    /* x3 = (xx1 - 1)^2 */
    curve25519_num_sub(&x3, &xx1, &p->z);
    curve25519_num_sqr(&x3, &x3);

    /* z3 = 4 * x * (xx1 + a * x + 1) */
    curve25519_num_mul(&z3, &kCurveA, &p->x);
    curve25519_num_add(&z3, &z3, &xx1);
    curve25519_num_add(&z3, &z3, &p->z);

    curve25519_num_mul(&z3, &z3, &p->x);
    curve25519_num_mul(&out->z, &z3, &kFour);

    curve25519_num_copy(&out->x, &x3);
  } else {
    /* http://hyperelliptic.org/EFD/g1p/auto-montgom-xz.html#doubling-dbl-1987-m-3 */
    curve25519_num_t a;
    curve25519_num_t b;
    curve25519_num_t c;

    /* a = x + z */
    curve25519_num_add(&a, &p->x, &p->z);

    /* a = a^2 */
    curve25519_num_sqr(&a, &a);

    /* b = x - z */
    curve25519_num_sub(&b, &p->x, &p->z);

    /* b = b^2 */
    curve25519_num_sqr(&b, &b);

    /* c = a - b */
    curve25519_num_sub(&c, &a, &b);

    /* x3 = a * b */
    curve25519_num_mul(&out->x, &a, &b);

    /* z3 = c * (b + a24 * c) */
    curve25519_num_mul(&out->z, &kCurveA24, &c);
    curve25519_num_add(&out->z, &out->z, &b);
    curve25519_num_mul(&out->z, &out->z, &c);
  }
  out->normalized = 0;
}


void curve25519_point_diff_add(curve25519_point_t* out,
                               const curve25519_point_t* p1,
                               const curve25519_point_t* p2,
                               const curve25519_point_t* diff) {
  /* http://hyperelliptic.org/EFD/g1p/auto-montgom-xz.html#diffadd-mdadd-1987-m */

  curve25519_num_t a;
  curve25519_num_t b;
  curve25519_num_t c;
  curve25519_num_t d;
  curve25519_num_t da;
  curve25519_num_t cb;

  /* a = x2 + z2 */
  curve25519_num_add(&a, &p1->x, &p1->z);

  /* b = x2 - z2 */
  curve25519_num_sub(&b, &p1->x, &p1->z);

  /* c = x3 + z3 */
  curve25519_num_add(&c, &p2->x, &p2->z);

  /* d = x3 - z3 */
  curve25519_num_sub(&d, &p2->x, &p2->z);

  /* da = d * a */
  curve25519_num_mul(&da, &d, &a);

  /* cb = c * b */
  curve25519_num_mul(&cb, &c, &b);

  if (diff->normalized) {
    /* z5 = x1 * (da - cb)^2 */
    curve25519_num_sub(&out->z, &da, &cb);
    curve25519_num_sqr(&out->z, &out->z);
    curve25519_num_mul(&out->z, &out->z, &diff->x);

    /* x5 = (da + cb)^2 */
    curve25519_num_add(&out->x, &da, &cb);
    curve25519_num_sqr(&out->x, &out->x);
  } else {
    /* z5 = x1 * (da - cb)^2 */
    curve25519_num_sub(&out->z, &da, &cb);
    curve25519_num_sqr(&out->z, &out->z);
    curve25519_num_mul(&out->z, &out->z, &diff->x);

    /* x5 = z1 * (da + cb)^2 */
    curve25519_num_add(&out->x, &da, &cb);
    curve25519_num_sqr(&out->x, &out->x);
    curve25519_num_mul(&out->x, &out->x, &diff->z);
  }
  out->normalized = 0;
}


void curve25519_point_normalize(curve25519_point_t* out) {
  curve25519_num_t zinv;

  if (out->normalized)
    return;

  curve25519_num_inv(&zinv, &out->z);
  curve25519_num_mul(&out->x, &out->x, &zinv);
  curve25519_num_one(&out->z);
  out->normalized = 1;
}
