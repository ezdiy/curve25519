#ifndef TEST_COMMON_H_
#define TEST_COMMON_H_

#include <string.h>

#include "curve25519.h"

#include "test-list.h"
#include "mini/test.h"

static void check_equal_data(uint8_t* actual, uint8_t* expected, size_t size,
                             const char* msg) {
  size_t i;

  if (memcmp(actual, expected, size) == 0)
    return;

  fprintf(stderr, "Failure: %s\n", msg);
  fprintf(stderr, "check_equal_data(\n  actual:   ");
  for (i = 0; i < size; i++)
    fprintf(stderr, "%02x", actual[i]);
  fprintf(stderr, "\n  expected: ");
  for (i = 0; i < size; i++)
    fprintf(stderr, "%02x", expected[i]);
  fprintf(stderr, "\n)\n");
  fflush(stderr);

  abort();
}

#endif  /* TEST_COMMON_H_ */
