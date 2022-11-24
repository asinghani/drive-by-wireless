#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "config.h"
#include "utils.h"
#include "testpoints.h"

void tp_init() {
    // Set up GPIO for test points
    for (int i = 0; i <= 6; i++) {
        gpio_init(i);
        gpio_set_dir(i, 1);
        gpio_put(i, 0);
    }

    // Cannot use tp_raise because scheduler isn't running yet
    for (int i = 1; i <= 6; i++) {
        gpio_put(i, 1);
        sleep_us(TP_TIME_US);
        gpio_put(i, 0);
    }
}

void tp_statusled(bool value) {
    gpio_put(IO_STATUS, value);
}

void tp_raise(int tp) {
    xassert((tp >= 1) && (tp <= 6));
    vTaskSuspendAll();
    gpio_put(tp, 1);
    sleep_us(TP_TIME_US);
    gpio_put(tp, 0);
    xTaskResumeAll();
}
