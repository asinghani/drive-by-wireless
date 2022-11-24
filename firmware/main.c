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

extern void cockpit_main();
extern void drivetrain_main();
extern void steering_main();

int main() {
    // Setup common peripherals
    xosc_init();
    stdio_init_all();
    tp_init();

    for (int i = 0; i <= 12; i++) {
        tp_statusled(i % 2);
        sleep_ms(200);
    }
    printf("Hello from zone %s\n", ZONE_NAME);

    xassert(portSUPPORT_SMP == 1);
    xassert(configNUM_CORES == 2);

    printf("Starting %s\n", ZONE_NAME);

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

