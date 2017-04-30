#ifndef INCLUDE_CURVE25519_H_
#define INCLUDE_CURVE25519_H_

#include <stdint.h>

typedef struct curve25519_ed_vanity_options_s curve22519_ed_vanity_options_t;

struct curve25519_ed_vanity_options_s {
  uint8_t point[32];

  uint8_t rnd[32];
  uint8_t postfix[32];

  unsigned int max_tries;
  unsigned int postfix_len;
};

/* Attempt to find such `out` that `out * Point` will have desired binary
 * `postfix` of `postfix_len` bit-length.
 *
 * Return values:
 *   *  1 - scalar multiplier found and stored in `out
 *   *  0 - scalar multiplier not found
 *   * -1 - error
 */
int curve25519_ed_vanity(uint8_t out[32], unsigned int* tries_made,
                         const curve22519_ed_vanity_options_t* options);

#endif  /* INCLUDE_CURVE25519_H_ */
