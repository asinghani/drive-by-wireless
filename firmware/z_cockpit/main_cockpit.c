#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include <stdio.h>
#include "peripherals/testpoints.h"

// Hardware setup
void cockpit_setup() {
    // TODO
}

static void cockpit_uwb_task(void *arg);
static void cockpit_wheel_read_task(void *arg);
static void cockpit_wheel_write_task(void *arg);

void cockpit_main() {
    printf("Start cockpit_main\n");
    cockpit_setup();

    TaskHandle_t uwb_task;
    xTaskCreate(cockpit_uwb_task, "cockpit_uwb_task",
            configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 3, &uwb_task);
    vTaskCoreAffinitySet(uwb_task, 2);

    TaskHandle_t wheel_read_task;
    xTaskCreate(cockpit_wheel_read_task, "cockpit_wheel_read_task",
            configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, &wheel_read_task);
    vTaskCoreAffinitySet(wheel_read_task, 1);

    TaskHandle_t wheel_write_task;
    xTaskCreate(cockpit_wheel_write_task, "cockpit_wheel_write_task",
            configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &wheel_write_task);
    vTaskCoreAffinitySet(wheel_write_task, 1);

    printf("Starting scheduler\n");
    vTaskStartScheduler();
}

static void cockpit_uwb_task(void *arg) {
	TickType_t tick = xTaskGetTickCount();

    while (true) {
        tp_statusled(1);
		vTaskDelayUntil(&tick, 500);
        tp_statusled(0);
		vTaskDelayUntil(&tick, 500);
    }
}

static void cockpit_wheel_read_task(void *arg) {
	TickType_t tick = xTaskGetTickCount();

    while (true) {
		vTaskDelayUntil(&tick, 500);
        printf("wheel read task\n");
    }
}

static void cockpit_wheel_write_task(void *arg) {
	TickType_t tick = xTaskGetTickCount();

    while (true) {
		vTaskDelayUntil(&tick, 500);
        printf("wheel write task\n");
    }
}

