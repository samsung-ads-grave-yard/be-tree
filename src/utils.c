#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "betree.h"
#include "utils.h"
#include "value.h"

int64_t d64min(int64_t a, int64_t b)
{
    return a < b ? a : b;
}

int64_t d64max(int64_t a, int64_t b)
{
    return a > b ? a : b;
}

uint64_t u64max(uint64_t a, uint64_t b)
{
    return a > b ? a : b;
}

size_t smin(size_t a, size_t b)
{
    return a < b ? a : b;
}

size_t smax(size_t a, size_t b)
{
    return a > b ? a : b;
}

bool feq(double a, double b)
{
    return fabs(a - b) < __DBL_EPSILON__;
}

bool fne(double a, double b)
{
    return !feq(a, b);
}

int icmpfunc(const void *a, const void *b) 
{
    const int64_t x = *(int64_t*)a;
    const int64_t y = *(int64_t*)b;
    if (x < y) {
        return -1;
    }
    if (x > y) {
        return 1;
    }
    return 0;
}

int scmpfunc(const void *a, const void *b) 
{
    const struct string_value* x = (struct string_value*)a;
    const struct string_value* y = (struct string_value*)b;
    if (x->str < y->str) {
        return -1;
    }
    if (x->str > y->str) {
        return 1;
    }
    return 0;
}

int iecmpfunc(const void *a, const void *b) 
{
    const struct integer_enum_value* x = (struct integer_enum_value*)a;
    const struct integer_enum_value* y = (struct integer_enum_value*)b;
    if (x->ienum < y->ienum) {
        return -1;
    }
    if (x->ienum > y->ienum) {
        return 1;
    }
    return 0;
}

