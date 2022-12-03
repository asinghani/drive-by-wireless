#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include <stdio.h>
#include <string.h>
#include "peripherals/testpoints.h"
#include "peripherals/drivetrain_servo.h"

struct DrivetrainState {
    int8_t motor_speed;
    bool brake_state;
    // TODO blinker
} shm_drivetrain;

// Hardware setup
void steering_setup() {
    memset(&shm_drivetrain, 0, sizeof(shm_drivetrain));
    drivetrain_motor_init();
}

static void drivetrain_uwb_task(void *arg);
static void drivetrain_motor_task(void *arg);

void drivetrain_main() {
    printf("Start steering_main\n");
    drivetrain_setup();

    TaskHandle_t uwb_task;
    xTaskCreate(drivetrain_uwb_task, "drivetrain_uwb_task",
            configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, &uwb_task);
    vTaskCoreAffinitySet(uwb_task, 2);

    TaskHandle_t motor_task;
    xTaskCreate(drivetrain_motor_task, "drivetrain_motor_task",
            configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, &servo_task);
    vTaskCoreAffinitySet(motor_task, 1);


    printf("Starting scheduler\n");
    vTaskStartScheduler();
}

static void drivetrain_uwb_task(void *arg) {
	TickType_t tick = xTaskGetTickCount();

    while (true) {
        tp_statusled(1);
		vTaskDelayUntil(&tick, 1000);
        tp_statusled(0);
		vTaskDelayUntil(&tick, 1000);
    }
}

static void drivetrain_motor_task(void *arg) {
	TickType_t tick = xTaskGetTickCount();

    while (true) {
		vTaskDelayUntil(&tick, 10);
        drivetrain_motor_set(shm_drivetrain.speed, shm_drivetrain.brake_state);
    }
}

