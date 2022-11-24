#ifndef UTILS_H
#define UTILS_H

// Use this instead of `assert` to get meaningful error messages over UART
#define xassert(cond) if (!(cond)) { _xassert_fail(#cond, __LINE__); }

void _xassert_fail(const char* cond, int line);

#endif
