#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include <stdio.h>
#include <string.h>
#include "peripherals/testpoints.h"

#include "pico/stdlib.h"
#include "pico/stdio.h"
#include "hardware/adc.h"
#include "config.h"

#include "decawave/uwb_config.h"
#include "decawave/uwb_library.h"
#include "config.h"
#include "utils.h"

struct DrivetrainState {
    uint8_t throttle;
    bool brake;

    // TODO blinker
} shm_drivetrain;

// Hardware setup
void drivetrain_setup() {
    memset(&shm_drivetrain, 0, sizeof(shm_drivetrain));
    // TODO

    uwb_init();
}

static void drivetrain_uwb_task(void *arg);
static void drivetrain_motor_task(void *arg);

void drivetrain_main() {
    printf("Start drivetrain_main\n");
    drivetrain_setup();

    TaskHandle_t uwb_task;
    xTaskCreate(drivetrain_uwb_task, "drivetrain_uwb_task",
            configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 3, &uwb_task);
    vTaskCoreAffinitySet(uwb_task, 2);

    TaskHandle_t motor_task;
    xTaskCreate(drivetrain_motor_task, "drivetrain_motor_task",
            configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, &motor_task);
    vTaskCoreAffinitySet(motor_task, 1);

    printf("Starting scheduler\n");
    vTaskStartScheduler();
}

static uint8_t rx_buffer[127];
static uint8_t tx_msg[125];

static void drivetrain_uwb_task(void *arg) {
	TickType_t tick = xTaskGetTickCount();

    printf("Start UWB follower\n");

    int ctr = 0;
    bool success;
    while (true) {
        memset(rx_buffer, 0, sizeof(rx_buffer));
        success = (receive_msg(rx_buffer) != -1);
        if (success && (rx_buffer[0] == ZID_COCKPIT) && (rx_buffer[1] == ZONE_ID)) {
            tx_msg[0] = ZONE_ID;
            tx_msg[1] = ZID_COCKPIT;
            send_msg(sizeof(tx_msg), tx_msg, 0);
        }

        ctr++;
    }

    bool up = false;
    while (true) {
        /*tp_statusled(1);
		vTaskDelayUntil(&tick, 1000);
        tp_statusled(0);
		vTaskDelayUntil(&tick, 1000);*/

		vTaskDelayUntil(&tick, 20);
    }
}

static void drivetrain_motor_task(void *arg) {
	TickType_t tick = xTaskGetTickCount();

    while (true) {
		vTaskDelayUntil(&tick, 10);
    }
}
