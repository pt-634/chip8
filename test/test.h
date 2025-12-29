#ifndef TEST_H
#define TEST_H

#include <stdio.h>

#define ASSERT(cond, msg) \
    do { if (!(cond)) { printf("FAIL: %s\n", msg); return 1; } } while (0)

#endif
