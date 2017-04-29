#include "include/curve25519.h"

#include "src/point.h"
#include "src/field.h"
#include "src/common.h"


void curve25519_point_init(curve25519_point_t* p, const uint8_t* x) {
  curve25519_num_from_bin(&p->x, x);
  curve25519_num_one(&p->z);
  p->normalized = 1;
}


/* TODO(indutny): rewrite for fast squaring */
void curve25519_point_dbl(curve25519_point_t* p) {
  if (p->normalized) {
    /* http://hyperelliptic.org/EFD/g1p/auto-montgom-xz.html#doubling-mdbl-1987-m */
    curve25519_num_t xx1;
  } else {
    /* http://hyperelliptic.org/EFD/g1p/auto-montgom-xz.html#doubling-dbl-1987-m-3 */
  }
}
