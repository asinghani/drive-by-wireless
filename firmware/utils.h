#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

// Use this instead of `assert` to get meaningful error messages over UART
#define xassert(cond) if (!(cond)) { _xassert_fail(#cond, __LINE__); }
void _xassert_fail(const char* cond, int line);

float absf(float x);

int clamp(int x, int lo, int hi);

// Signed for easier use of negative offsets
int32_t millis();

#endif
