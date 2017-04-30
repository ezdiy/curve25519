#ifndef TEST_COMMON_H_
#define TEST_COMMON_H_

#include <string.h>

#include "curve25519.h"

#include "src/common.h"
#include "src/field.h"
#include "src/point.h"
#include "src/edpoint.h"

#include "test-list.h"
#include "mini/test.h"

typedef struct test_field_vector_s test_field_vector_t;
typedef struct test_field_single_vector_s test_field_single_vector_t;
typedef struct test_point_s test_point_t;
typedef struct test_point_dbl_vector_s test_point_dbl_vector_t;
typedef struct test_point_diff_add_vector_s test_point_diff_add_vector_t;
typedef struct test_ed_point_unpack_vector_s test_ed_point_unpack_vector_t;

struct test_field_vector_s {
  uint8_t a[32];
  uint8_t b[32];
  uint8_t expected[32];
  const char* description;
};

struct test_field_single_vector_s {
  uint8_t num[32];
  uint8_t expected[32];
  const char* description;
};

struct test_point_s {
  uint8_t x[32];
  uint8_t z[32];
};

struct test_point_dbl_vector_s {
  test_point_t p;
  test_point_t expected;
  const char* description;
};

struct test_point_diff_add_vector_s {
  test_point_t p1;
  test_point_t p2;
  test_point_t diff;
  test_point_t expected;
  const char* description;
};

struct test_ed_point_unpack_vector_s {
  uint8_t bin[32];
  struct {
    uint8_t x[32];
    uint8_t y[32];
  } expected;
  const char* description;
};

static void check_equal_data(uint8_t* actual, const uint8_t* expected,
                             size_t size, const char* msg) {
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
