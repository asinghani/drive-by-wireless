#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/stdio.h"

#include "utils.h"
#include "config.h"
#include "peripherals/testpoints.h"

#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "hardware/xosc.h"
#include "hardware/watchdog.h"

extern void cockpit_main();
extern void drivetrain_main();
extern void steering_main();

int main() {
    // Setup common peripherals
    busy_wait_ms(100);
    xosc_init();
    stdio_init_all();
    tp_init();

    watchdog_enable(500, 1);

    for (int i = 0; i <= 4; i++) {
        tp_statusled(i % 2);
        busy_wait_ms(50);
    }
    printf("Hello from zone %s\n", ZONE_NAME);

    xassert(portSUPPORT_SMP == 1);
    xassert(configNUM_CORES == 2);

#ifdef ZONE_COCKPIT
    cockpit_main();
#endif

#ifdef ZONE_DRIVETRAIN
    drivetrain_main();
#endif

#ifdef ZONE_STEERING
    steering_main();
#endif

    xassert(!"Should never return from scheduler. Scheduling failed.");
    return 0;
}

