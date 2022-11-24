#include <stdio.h>
#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "utils.h"

void _xassert_fail(const char* cond, int line) {
    while (1) {
        printf("Assertion '%s' failed on line %d. Please add more magic.\n",
                cond, line);
        sleep_ms(2000);
    }
}

