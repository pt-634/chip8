#ifndef TEST_H
#define TEST_H

#define ASSERT(cond, msg) \
    do { if (!(cond)) { printf("FAIL: %s\n", msg); return 1; } } while (0)

#endif
