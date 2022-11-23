#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/stdio.h"
#include "hardware/uart.h"
#include "hardware/xosc.h"

#include "steering_wheel.h"

int main() {
    xosc_init();
    stdio_init_all();
    sleep_ms(3000);
    printf("test\n");

    gpio_init(0);
    gpio_set_dir(0, true);

    while (1) {
        gpio_put(0, 0);
        sleep_ms(500);
        gpio_put(0, 1);
        sleep_ms(500);
    }

    steering_setup();

    while (1) {
        steering_update();
        steering_send_feedback(steering_get_brake() ? 0 : (steering_get_angle()));
        sleep_ms(50);
    }


    while (1);
    return 0;
}
