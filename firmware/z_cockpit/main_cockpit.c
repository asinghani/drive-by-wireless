#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include <stdio.h>
#include <string.h>
#include "peripherals/testpoints.h"
#include "peripherals/steering_wheel.h"

struct CockpitState {
    int8_t angle;
    uint8_t throttle;
    bool btn_brake;
    bool btn_BL;
    bool btn_BR;

    int8_t feedback;

    bool blinker_left;
    bool blinker_right;
} shm_cockpit;

// Hardware setup
void cockpit_setup() {
    memset(&shm_cockpit, 0, sizeof(shm_cockpit));
    wheel_setup();
}

static void cockpit_uwb_task(void *arg);
static void cockpit_wheel_read_task(void *arg);
static void cockpit_wheel_write_task(void *arg);
static void cockpit_blinker_fsm_task(void *arg);

void cockpit_main() {
    printf("Start cockpit_main\n");
    cockpit_setup();

    TaskHandle_t uwb_task;
    xTaskCreate(cockpit_uwb_task, "cockpit_uwb_task",
            configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 4, &uwb_task);
    vTaskCoreAffinitySet(uwb_task, 2);

    TaskHandle_t wheel_read_task;
    xTaskCreate(cockpit_wheel_read_task, "cockpit_wheel_read_task",
            configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 3, &wheel_read_task);
    vTaskCoreAffinitySet(wheel_read_task, 1);

    TaskHandle_t wheel_write_task;
    xTaskCreate(cockpit_wheel_write_task, "cockpit_wheel_write_task",
            configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, &wheel_write_task);
    vTaskCoreAffinitySet(wheel_write_task, 1);

    TaskHandle_t blinker_fsm_task;
    xTaskCreate(cockpit_blinker_fsm_task, "cockpit_blinker_fsm_task",
            configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &blinker_fsm_task);
    vTaskCoreAffinitySet(blinker_fsm_task, 1);

    printf("Starting scheduler\n");
    vTaskStartScheduler();
}

static void cockpit_uwb_task(void *arg) {
	TickType_t tick = xTaskGetTickCount();

    while (true) {
        tp_statusled(shm_cockpit.btn_brake);
        shm_cockpit.feedback = shm_cockpit.btn_brake ? 0 : shm_cockpit.angle;
		vTaskDelayUntil(&tick, 20);
    }
}

static void cockpit_wheel_read_task(void *arg) {
	TickType_t tick = xTaskGetTickCount();

    while (true) {
		vTaskDelayUntil(&tick, 20);
        wheel_update();

        shm_cockpit.angle = wheel_get_angle();
        shm_cockpit.throttle = wheel_get_throttle();
        shm_cockpit.btn_brake = wheel_get_brake();
        shm_cockpit.btn_BL = wheel_get_BL();
        shm_cockpit.btn_BR = wheel_get_BR();
    }
}

static void cockpit_wheel_write_task(void *arg) {
	TickType_t tick = xTaskGetTickCount();

    while (true) {
		vTaskDelayUntil(&tick, 20);
        //printf("FEEDBACK %d\n", shm_cockpit.feedback);
        wheel_send_feedback(shm_cockpit.feedback);
    }
}

static void cockpit_blinker_fsm_task(void *arg) {
	TickType_t tick = xTaskGetTickCount();

    while (true) {
        //printf("angle %d\n", shm_cockpit.angle);
		vTaskDelayUntil(&tick, 100);
    }
}
