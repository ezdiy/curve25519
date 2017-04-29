#include <string.h>  /* memset */

#include "src/field.h"
#include "src/common.h"

static const curve25519_num_t kPrime = {
  .limbs = {
    0xffffffffffffffedLLU,
    0xffffffffffffffffLLU,
    0xffffffffffffffffLLU,
    0x7fffffffffffffffLLU
  }
};


static const curve25519_num_t kOne = {
  .limbs = { 1, 0, 0, 0 }
};


static const curve25519_num_t kZero = {
  .limbs = { 0, 0, 0, 0 }
};


void curve25519_num_add(curve25519_num_t* out, const curve25519_num_t* num) {
  uint64_t* olimbs = out->limbs;
  const uint64_t* nlimbs = num->limbs;

  __asm__ __volatile__ (
      /* load all limbs */
      "movq 0(%0), %%r8\n"
      "movq 8(%0), %%r9\n"
      "movq 16(%0), %%r10\n"
      "movq 24(%0), %%r11\n"

      /* Prepare storage */
      "xorq %%rax, %%rax\n"

      /* add limbs with carry */
      "addq 0(%1), %%r8\n"
      "adcq 8(%1), %%r9\n"
      "adcq 16(%1), %%r10\n"
      "adcq 24(%1), %%r11\n"

      /* A + B = X * 2^N + Y = 2 * K * X + Y */
      "adcq $0, %%rax\n"
      "movq $38, %%rdx\n"
      "mulq %%rdx\n"

      /* Add 2 * K * X */
      "addq %%rax, %%r8\n"
      "adcq $0, %%r9\n"
      "adcq $0, %%r10\n"
      "adcq $0, %%r11\n"

      /* store all limbs */
      "movq %%r8, 0(%0)\n"
      "movq %%r9, 8(%0)\n"
      "movq %%r10, 16(%0)\n"
      "movq %%r11, 24(%0)\n"
  : "+r" (olimbs)
  : "r" (nlimbs)
  : "%rax", "%rdx", "%r8", "%r9", "%r10", "%r11", "cc", "memory");
}


void curve25519_num_sub(curve25519_num_t* out, const curve25519_num_t* num) {
  uint64_t* olimbs = out->limbs;
  const uint64_t* nlimbs = num->limbs;

  __asm__ __volatile__ (
      /* load all limbs */
      "movq 0(%0), %%r8\n"
      "movq 8(%0), %%r9\n"
      "movq 16(%0), %%r10\n"
      "movq 24(%0), %%r11\n"

      /* Prepare storage */
      "xorq %%rax, %%rax\n"

      /* sub limbs with borrow */
      "subq 0(%1), %%r8\n"
      "sbbq 8(%1), %%r9\n"
      "sbbq 16(%1), %%r10\n"
      "sbbq 24(%1), %%r11\n"

      /* Check overflow */
      "adc $0, %%rax\n"
      "movq $38, %%rdx\n"
      "mulq %%rdx\n"

      /* Sub 2 * K * X */
      "subq %%rax, %%r8\n"
      "sbbq $0, %%r9\n"
      "sbbq $0, %%r10\n"
      "sbbq $0, %%r11\n"

      /* store all limbs */
      "movq %%r8, 0(%0)\n"
      "movq %%r9, 8(%0)\n"
      "movq %%r10, 16(%0)\n"
      "movq %%r11, 24(%0)\n"
  : "+r" (olimbs)
  : "r" (nlimbs)
  : "%rax", "%rdx", "%r8", "%r9", "%r10", "%r11", "cc", "memory");
}


void curve25519_num_mul(curve25519_num_t* out,
                        const curve25519_num_t* num) {
  uint64_t* olimbs = out->limbs;
  const uint64_t* nlimbs = num->limbs;

  __asm__ __volatile__ (
      /* tmp = (r15, r14, r13) = (t3, t2, t1, t0) */
      /* output = (rcx, r12, r11, r10, r9, r8) = (oc1, oc0, o3, o2, o1, o0) */
      /* a = (a3, a2, a1, a0) = out */
      /* b = (b3, b2, b1, b0) = num */

      /* (o2, o1, o0) = a0 * b0 + 38 * (a1 * b3 + a2 * b2 + a3 * b1) */

      /* (o1, o0) = a1 * b3 */
      "movq 8(%0), %%rax\n"
      "mulq 24(%1)\n"
      "movq %%rax, %%r8\n"
      "movq %%rdx, %%r9\n"

      /* (o2, o1, o0) += a2 * b2 */
      "xorq %%r10, %%r10\n"
      "movq 16(%0), %%rax\n"
      "mulq 16(%1)\n"
      "addq %%rax, %%r8\n"
      "adcq %%rdx, %%r9\n"
      "adcq $0, %%r10\n"

      /* (o2, o1, o0) += a3 * b1 */
      "movq 24(%0), %%rax\n"
      "mulq 8(%1)\n"
      "addq %%rax, %%r8\n"
      "adcq %%rdx, %%r9\n"
      "adcq $0, %%r10\n"

      /* (o2, o1, o0) *= 38 */
      "movq $38, %%rax\n"
      "mulq %%r10\n"
      "movq %%rax, %%r10\n"

      "movq $38, %%rax\n"
      "mulq %%r9\n"
      "movq %%rax, %%r9\n"
      "addq %%rdx, %%r10\n"

      "movq $38, %%rax\n"
      "mulq %%r8\n"
      "movq %%rax, %%r8\n"
      "addq %%rdx, %%r9\n"
      "adcq $0, %%r10\n"

      /* (o2, o1, o0) += a0 * b0 */
      "movq 0(%0), %%rax\n"
      "mulq 0(%1)\n"
      "addq %%rax, %%r8\n"
      "adcq %%rdx, %%r9\n"
      "adcq $0, %%r10\n"

      /* (t2, t1, t0) = a0 * b1 + a1 * b0 + 38 * (a2 * b3 + a3 * b2) */

      /* (t1, t0) = a2 * b3 */
      "movq 16(%0), %%rax\n"
      "mulq 24(%1)\n"
      "movq %%rax, %%r13\n"
      "movq %%rdx, %%r14\n"

      /* (t2, t1, t0) += a3 * b2 */
      "xorq %%r15, %%r15\n"
      "movq 24(%0), %%rax\n"
      "mulq 16(%1)\n"
      "addq %%rax, %%r13\n"
      "adcq %%rdx, %%r14\n"
      "adcq $0, %%r15\n"

      /* (t2, t1, t0) *= 38 */
      "movq $38, %%rax\n"
      "mulq %%r15\n"
      "movq %%rax, %%r15\n"

      "movq $38, %%rax\n"
      "mulq %%r14\n"
      "movq %%rax, %%r14\n"
      "addq %%rdx, %%r15\n"

      "movq $38, %%rax\n"
      "mulq %%r13\n"
      "movq %%rax, %%r13\n"
      "addq %%rdx, %%r14\n"
      "adcq $0, %%r15\n"

      /* (t2, t1, t0) += a0 * b1 */
      "movq 0(%0), %%rax\n"
      "mulq 8(%1)\n"
      "addq %%rax, %%r13\n"
      "adcq %%rdx, %%r14\n"
      "adcq $0, %%r15\n"

      /* (t2, t1, t0) += a1 * b0 */
      "movq 8(%0), %%rax\n"
      "mulq 0(%1)\n"
      "addq %%rax, %%r13\n"
      "adcq %%rdx, %%r14\n"
      "adcq $0, %%r15\n"

      /* (o3, o2, o1) += (t2, t1, t0) */
      "xorq %%r11, %%r11\n"
      "addq %%r13, %%r9\n"
      "adcq %%r14, %%r10\n"
      "adcq %%r15, %%r11\n"

      /* (t2, t1, t0) = a0 * b2 + a1 * b1 + a2 * b0 +
       *                38 * a3 * b3 */

      /* (t1, t0) = a3 * b3 */
      "movq 24(%0), %%rax\n"
      "mulq 24(%1)\n"
      "movq %%rax, %%r13\n"
      "movq %%rdx, %%r14\n"

      /* (t2, t1, t0) *= 38 */
      "movq $38, %%rax\n"
      "mulq %%r14\n"
      "movq %%rax, %%r14\n"
      "movq %%rdx, %%r15\n"

      "movq $38, %%rax\n"
      "mulq %%r13\n"
      "movq %%rax, %%r13\n"
      "addq %%rdx, %%r14\n"
      "adcq $0, %%r15\n"

      /* (t2, t1, t0) += a0 * b2 */
      "movq 0(%0), %%rax\n"
      "mulq 16(%1)\n"
      "addq %%rax, %%r13\n"
      "adcq %%rdx, %%r14\n"
      "adcq $0, %%r15\n"

      /* (t2, t1, t0) += a1 * b1 */
      "movq 8(%0), %%rax\n"
      "mulq 8(%1)\n"
      "addq %%rax, %%r13\n"
      "adcq %%rdx, %%r14\n"
      "adcq $0, %%r15\n"

      /* (t2, t1, t0) += a2 * b0 */
      "movq 16(%0), %%rax\n"
      "mulq 0(%1)\n"
      "addq %%rax, %%r13\n"
      "adcq %%rdx, %%r14\n"
      "adcq $0, %%r15\n"

      /* (oc0, o3, o2) += (t2, t1, t0) */
      "xorq %%r12, %%r12\n"
      "addq %%r13, %%r10\n"
      "adcq %%r14, %%r11\n"
      "adcq %%r15, %%r12\n"

      /* (t2, t1, t0) = a0 * b3 + a1 * b2 + a2 * b1 + a3 * b0 */

      /* (t1, t0) = a0 * b3 */
      "movq 0(%0), %%rax\n"
      "mulq 24(%1)\n"
      "movq %%rax, %%r13\n"
      "movq %%rdx, %%r14\n"

      /* (t2, t1, t0) += a1 * b2 */
      "xorq %%r15, %%r15\n"
      "movq 8(%0), %%rax\n"
      "mulq 16(%1)\n"
      "addq %%rax, %%r13\n"
      "adcq %%rdx, %%r14\n"
      "adcq $0, %%r15\n"

      /* (t2, t1, t0) += a2 * b1 */
      "movq 16(%0), %%rax\n"
      "mulq 8(%1)\n"
      "addq %%rax, %%r13\n"
      "adcq %%rdx, %%r14\n"
      "adcq $0, %%r15\n"

      /* (t2, t1, t0) += a3 * b0 */
      "movq 24(%0), %%rax\n"
      "mulq 0(%1)\n"
      "addq %%rax, %%r13\n"
      "adcq %%rdx, %%r14\n"
      "adcq $0, %%r15\n"

      /* (oc1, oc0, o3) += (t2, t1, t0) */
      "xorq %%rcx, %%rcx\n"
      "addq %%r13, %%r11\n"
      "adcq %%r14, %%r12\n"
      "adcq %%r15, %%rcx\n"

      /* (t2, t1, t0) = (oc1, oc0) * 38 */
      "movq $38, %%rax\n"
      "mulq %%rcx\n"
      "movq %%rax, %%r14\n"
      "movq %%rdx, %%r15\n"

      "movq $38, %%rax\n"
      "mulq %%r12\n"
      "movq %%rax, %%r13\n"
      "addq %%rdx, %%r14\n"
      "adcq $0, %%r15\n"

      /* (o3, o2, o1, o0) += (t2, t1, t0) */
      "addq %%r13, %%r8\n"
      "adcq %%r14, %%r9\n"
      "adcq %%r15, %%r10\n"
      "adcq $0, %%r11\n"

      /* store limbs */
      "movq %%r8, 0(%0)\n"
      "movq %%r9, 8(%0)\n"
      "movq %%r10, 16(%0)\n"
      "movq %%r11, 24(%0)\n"
  : "+r" (olimbs)
  : "r" (nlimbs)
  : "%rax", "%rcx", "%rdx",
    "%r8", "%r9", "%r10", "%r11", "%r12", "%r13", "%r14", "%r15",
    "cc", "memory");
}


/* TODO(indutny): implement me and write tests */
void curve25519_num_sqr(curve25519_num_t* out) {
  curve25519_num_mul(out, out);
}


static void curve25519_num__shr(curve25519_num_t* num, uint8_t shift) {
  const uint64_t* nlimbs = num->limbs;

  __asm__ __volatile__ (
      /* Load all limbs */
      "movq 0(%0), %%r8\n"
      "movq 8(%0), %%r9\n"
      "movq 16(%0), %%r10\n"
      "movq 24(%0), %%r11\n"

      /* Shift */
      "movb %1, %%cl\n"
      "shrdq %%cl, %%r9, %%r8\n"
      "shrdq %%cl, %%r10, %%r9\n"
      "shrdq %%cl, %%r11, %%r10\n"
      "shrq %%cl, %%r11\n"

      /* Store all limbs */
      "movq %%r8, 0(%0)\n"
      "movq %%r9, 8(%0)\n"
      "movq %%r10, 16(%0)\n"
      "movq %%r11, 24(%0)\n"
  : "+r" (nlimbs)
  : "rm" (shift)
  : "cc", "r8", "r9", "r10", "r11", "cl", "memory");
}


static uint8_t curve25519_num__tzcnt(const curve25519_num_t* num) {
  uint64_t out;
  const uint64_t* nlimbs = num->limbs;

  __asm__ __volatile__ (
      "tzcnt 0(%1), %0\n"
  : "=r" (out)
  : "r" (nlimbs)
  : "cc", "memory");

  return (uint8_t) out;
}


static void curve25519_num__shift_both(curve25519_num_t* num,
                                       curve25519_num_t* t) {
  uint8_t shift_num;

  shift_num = curve25519_num__tzcnt(num);
  if (shift_num == 0)
    return;

  curve25519_num__shr(num, shift_num);

  for (;;) {
    uint8_t shift_t = curve25519_num__tzcnt(t);
    if (shift_t >= shift_num) {
      curve25519_num__shr(t, shift_num);
      break;
    }

    curve25519_num__shr(t, shift_t);
    shift_num -= shift_t;
    curve25519_num_add(t, &kPrime);
  }
}


static int curve25519_num__is_zero(const curve25519_num_t* num) {
  return (num->limbs[0] | num->limbs[1] | num->limbs[2] | num->limbs[3]) == 0;
}


/* NOTE: `out` muste be bigger or equal than num */
void curve25519_num_fast_sub(curve25519_num_t* out,
                             const curve25519_num_t* num) {
  uint64_t* olimbs = out->limbs;
  const uint64_t* nlimbs = num->limbs;

  __asm__ __volatile__ (
      /* load all limbs */
      "movq 0(%1), %%r8\n"
      "movq 8(%1), %%r9\n"
      "movq 16(%1), %%r10\n"
      "movq 24(%1), %%r11\n"

      /* sub limbs */
      "subq %%r8, 0(%0)\n"
      "sbbq %%r9, 8(%0)\n"
      "sbbq %%r10, 16(%0)\n"
      "sbbq %%r11, 24(%0)\n"
  : "+r" (olimbs)
  : "r" (nlimbs)
  : "%r8", "%r9", "%r10", "%r11", "cc", "memory");
}


void curve25519_num_inv(curve25519_num_t* out) {
  curve25519_num_t a;
  curve25519_num_t b;
  curve25519_num_t t0;
  curve25519_num_t t1;

  /* Extended Euclidean Algorithm */
  curve25519_num_normalize(out);

  curve25519_num_copy(&a, &kPrime);
  curve25519_num_copy(&b, out);
  curve25519_num_copy(&t0, &kZero);
  curve25519_num_copy(&t1, &kOne);

  if (curve25519_num__is_zero(&b))
    return curve25519_num_copy(out, &t1);

  for (;;) {
    curve25519_num__shift_both(&a, &t0);
    curve25519_num__shift_both(&b, &t1);

    if (curve25519_num_cmp(&a, &b) >= 0) {
      curve25519_num_fast_sub(&a, &b);
      curve25519_num_sub(&t0, &t1);

      if (curve25519_num__is_zero(&a))
        return curve25519_num_copy(out, &t1);
    } else {
      curve25519_num_fast_sub(&b, &a);
      curve25519_num_sub(&t1, &t0);

      /* XXX(indutny): can it ever get here? */
      /* TODO(indutny): write a test case */
      if (curve25519_num__is_zero(&b))
        return curve25519_num_copy(out, &t0);
    }
  }
}


/* Helpers */


/* TODO(indutny): make it constant-time, if ever desired */
void curve25519_num_normalize(curve25519_num_t* out) {
  if (curve25519_num_cmp(out, &kPrime) < 0)
    return;

  curve25519_num_sub(out, &kPrime);
}


void curve25519_num_copy(curve25519_num_t* out, const curve25519_num_t* num) {
  memcpy(out->limbs, num->limbs, sizeof(num->limbs));
}


int curve25519_num_cmp(const curve25519_num_t* a, const curve25519_num_t* b) {
  int i;
  for (i = (int) ARRAY_SIZE(a->limbs) - 1; i >= 0; i--)
    if (a->limbs[i] > b->limbs[i])
      return 1;
    else if (a->limbs[i] < b->limbs[i])
      return -1;
  return 0;
}


void curve25519_num_to_bin(uint8_t out[32], curve25519_num_t* num) {
  unsigned int i;
  unsigned int off;

  curve25519_num_normalize(num);

  /* TODO(indutny): which endianess is needed? */
  off = 0;
  for (i = 0; i < ARRAY_SIZE(num->limbs); i++) {
    uint64_t limb;
    unsigned int j;

    limb = num->limbs[i];
    for (j = 0; j < 8; j++) {
      out[off++] = limb & 0xff;
      limb >>= 8;
    }
  }
}


void curve25519_num_from_bin(curve25519_num_t* out, uint8_t bin[32]) {
  unsigned int i;
  unsigned int off;

  memset(out, 0, sizeof(*out));

  /* TODO(indutny): which endianess is needed? */
  off = 32;
  for (i = 0; i < 4; i++) {
    uint64_t limb;
    unsigned int j;

    limb = 0;
    for (j = 0; j < 8; j++) {
      limb <<= 8;
      limb |= bin[--off];
    }
    out->limbs[3 - i] = limb;
  }
}
