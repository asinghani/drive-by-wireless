#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include <stdio.h>
#include <string.h>
#include "peripherals/testpoints.h"
#include "peripherals/steering_servo.h"

struct SteeringState {
    int8_t angle;
    int8_t feedback;

    // TODO blinker
} shm_steering;

// Hardware setup
void steering_setup() {
    memset(&shm_steering, 0, sizeof(shm_steering));
    steering_servo_init();
}

static void steering_uwb_task(void *arg);
static void steering_servo_task(void *arg);
static void steering_feedback_task(void *arg);

void steering_main() {
    printf("Start steering_main\n");
    steering_setup();

    TaskHandle_t uwb_task;
    xTaskCreate(steering_uwb_task, "steering_uwb_task",
            configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 3, &uwb_task);
    vTaskCoreAffinitySet(uwb_task, 2);

    TaskHandle_t servo_task;
    xTaskCreate(steering_servo_task, "steering_servo_task",
            configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, &servo_task);
    vTaskCoreAffinitySet(servo_task, 1);

    TaskHandle_t feedback_task;
    xTaskCreate(steering_feedback_task, "steering_feedback_task",
            configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &feedback_task);
    vTaskCoreAffinitySet(feedback_task, 1);

    printf("Starting scheduler\n");
    vTaskStartScheduler();
}

static void steering_uwb_task(void *arg) {
	TickType_t tick = xTaskGetTickCount();

    while (true) {
        tp_statusled(1);
		vTaskDelayUntil(&tick, 1000);
        tp_statusled(0);
		vTaskDelayUntil(&tick, 1000);
    }
}

static void steering_servo_task(void *arg) {
	TickType_t tick = xTaskGetTickCount();

    while (true) {
		vTaskDelayUntil(&tick, 10);
        steering_servo_set(shm_steering.angle);
    }
}

static void steering_feedback_task(void *arg) {
	TickType_t tick = xTaskGetTickCount();

    bool up = false;
    while (true) {
		vTaskDelayUntil(&tick, 20);
        shm_steering.angle += up ? 2 : -2;
        if (shm_steering.angle >= 100) up = false;
        if (shm_steering.angle <= -100) up = true;
    }
}
