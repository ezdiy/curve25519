#include <string.h>

#include "include/curve25519.h"

static const uint64_t kPrime[] = {
  0xffffffffffffffedLLU,
  0xffffffffffffffffLLU,
  0xffffffffffffffffLLU,
  0x7fffffffffffffffLLU
};

/* TODO(indutny): make it constant-time, probably */
void curve25519_num_normalize(curve25519_num_t* out) {
  if (out->limbs[3] != kPrime[3] || out->limbs[2] != kPrime[2] ||
      out->limbs[1] != kPrime[1]) {
    return;
  }

  if (out->limbs[0] < kPrime[0])
    return;

  out->limbs[3] = 0;
  out->limbs[2] = 0;
  out->limbs[1] = 0;
  out->limbs[0] -= kPrime[0];
}


void curve25519_num_add(curve25519_num_t* out, const curve25519_num_t* num) {
  uint64_t* olimbs = out->limbs;
  const uint64_t* nlimbs = num->limbs;

  __asm__ volatile (
      /* load all limbs */
      "movq 0(%0), %%r8\n"
      "movq 8(%0), %%r9\n"
      "movq 16(%0), %%r10\n"
      "movq 24(%0), %%r11\n"

      /* add limbs with carry */
      "addq 0(%1), %%r8\n"
      "adcq 8(%1), %%r9\n"
      "adcq 16(%1), %%r10\n"
      "adcq 24(%1), %%r11\n"

      /* A - B = X * 2^N + Y = K * X + Y */
      "xorq %%rax, %%rax\n"
      "btrq $63, %%r11\n"
      "adc $0, %%rax\n"
      "movq $19, %%rdx\n"
      "mulq %%rdx\n"

      /* Add K * X */
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
  : "r" (nlimbs) :
    "%rax", "%rdx", "%r8", "%r9", "%r10", "%r11", "cc", "memory");
}


void curve25519_num_sub(curve25519_num_t* out, const curve25519_num_t* num) {
  uint64_t* olimbs = out->limbs;
  const uint64_t* nlimbs = num->limbs;

  __asm__ volatile (
      /* load all limbs */
      "movq 0(%0), %%r8\n"
      "movq 8(%0), %%r9\n"
      "movq 16(%0), %%r10\n"
      "movq 24(%0), %%r11\n"

      /* sub limbs with borrow */
      "subq 0(%1), %%r8\n"
      "sbbq 8(%1), %%r9\n"
      "sbbq 16(%1), %%r10\n"
      "sbbq 24(%1), %%r11\n"

      /* Check overflow */
      "xorq %%rax, %%rax\n"
      "btrq $63, %%r11\n"
      "adc $0, %%rax\n"
      "movq $19, %%rdx\n"
      "mulq %%rdx\n"

      /* Sub K * X */
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
  : "r" (nlimbs) :
    "%rax", "%rdx", "%r8", "%r9", "%r10", "%r11", "cc", "memory");
}


void curve25519_num_to_bin(uint8_t out[32], curve25519_num_t* num) {
  unsigned int i;
  unsigned int off;

  curve25519_num_normalize(num);

  /* TODO(indutny): which endianess is needed? */
  off = 0;
  for (i = 0; i < 4; i++) {
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
