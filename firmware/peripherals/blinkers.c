#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "config.h"
#include "utils.h"
#include "blinkers.h"

int32_t _blinker_basis_ts = 0;
bool _blinker_left = false;
bool _blinker_right = false;

void blinkers_init() {
    gpio_init(IO_BLINKL);
    gpio_set_dir(IO_BLINKL, 1);
    gpio_put(IO_BLINKL, 0);

    gpio_init(IO_BLINKR);
    gpio_set_dir(IO_BLINKR, 1);
    gpio_put(IO_BLINKR, 0);
}

// Update the blinker state asynchronously
void blinkers_update(int32_t offset, int32_t basis_ts, bool blink_left, bool blink_right, bool is_error_state) {
    int32_t global_ts = millis() - offset;

    if (is_error_state) {
        int64_t state = global_ts;
        state = state + 4300000000L; // handle negative time offsets
        state = (state / ERROR_BLINKER_RATE_MS);
        state = state & 1;
        gpio_put(IO_BLINKL, state);
        gpio_put(IO_BLINKR, state);

    } else {
        // Basis is used for two different things:
        // synchronizing new state changes,
        // and synchronizing blinks
        if (global_ts >= basis_ts) {
            _blinker_basis_ts = basis_ts;
            _blinker_left = blink_left;
            _blinker_right = blink_right;
        }

        int64_t state = global_ts - _blinker_basis_ts;
        state = state + 4300000000L; // handle negative time offsets
        state = (state / BLINKER_RATE_MS);
        state = !(state & 1); // Start blinking at basis time

        if (_blinker_left) {
            gpio_put(IO_BLINKL, state);
            gpio_put(IO_BLINKR, 0);
        } else if (_blinker_right) {
            gpio_put(IO_BLINKL, 0);
            gpio_put(IO_BLINKR, state);
        } else {
            gpio_put(IO_BLINKL, 0);
            gpio_put(IO_BLINKR, 0);
        }
    }
}
