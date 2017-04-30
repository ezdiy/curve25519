#include "include/curve25519.h"

#include <string.h>

#include "src/common.h"
#include "src/edpoint.h"


static int curve25519_ed_check_postfix(curve25519_ed_point_t* p,
                                       const uint8_t postfix[32],
                                       unsigned int postfix_len) {
  uint8_t bin[32];
  unsigned int left;
  unsigned int i;
  uint8_t mask;

  curve25519_ed_point_to_bin(bin, p);

  i = 31;
  left = postfix_len;
  while (left >= 8) {
    if (postfix[i] != bin[i])
      return 0;
    i--;
    left -= 8;
  }

  if (left == 0)
    return 0;

  mask = (1 << left) - 1;
  return ((postfix[i] ^ bin[i]) & mask) == 0;
}


int curve25519_ed_vanity(uint8_t out[32], unsigned int* tries_made,
                         const curve22519_ed_vanity_options_t* options) {
  curve25519_ed_point_t point;
  curve25519_ed_point_t base;
  unsigned int i;
  unsigned int max_tries;
  int result;

  /* Invalid `postfix_len` */
  if (options->postfix_len >= sizeof(options->postfix) * 8)
    return -1;

  if (curve25519_ed_point_from_bin(&point, options->point) != 0)
    return -1;

  result = 0;

  /* NOTE: `memmove` in case if they overlap or are the same */
  memcpy(out, options->rnd, sizeof(options->rnd));
  curve25519_ed_point_copy(&base, &point);
  curve25519_ed_point_scalar_mul(&point, &point, out);

  max_tries = options->max_tries;
  for (i = 0; i < max_tries; i++) {
    unsigned int j;
    uint8_t carry;

    if (curve25519_ed_check_postfix(&point, options->postfix,
                                    options->postfix_len)) {
      result = 1;
      goto done;
    }

    curve25519_ed_point_add(&point, &point, &base);
    carry = 1;
    for (j = 0; j < 32; j++) {
      uint8_t sum;

      sum = out[j] + carry;
      out[j] = sum;

      carry = sum == 0 ? 1 : 0;
      if (carry == 0)
        break;
    }

    /* Overflow, can't continue */
    if (carry)
      goto done;
  }

done:
  *tries_made = i;
  return result;
}
