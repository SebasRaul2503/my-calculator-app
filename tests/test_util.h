#ifndef TEST_UTIL_H
#define TEST_UTIL_H

#include <stdio.h>
#include <math.h>

static int test_failures = 0;

#define CHECK(cond) do { \
    if (!(cond)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        test_failures++; \
    } \
} while (0)

#define CHECK_DBL(a, b) do { \
    double _a = (a), _b = (b); \
    if (fabs(_a - _b) > 1e-9) { \
        fprintf(stderr, "FAIL %s:%d: %g != %g\n", __FILE__, __LINE__, _a, _b); \
        test_failures++; \
    } \
} while (0)

#define TEST_REPORT() return test_failures ? 1 : 0

#endif
