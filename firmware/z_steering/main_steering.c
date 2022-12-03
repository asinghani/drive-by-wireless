#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include <stdio.h>
#include <string.h>
#include "peripherals/testpoints.h"
#include "peripherals/steering_servo.h"

#include "pico/stdlib.h"
#include "pico/stdio.h"
#include "hardware/adc.h"
#include "config.h"

#include "decawave/uwb_config.h"
#include "decawave/uwb_library.h"
#include "config.h"
#include "utils.h"

struct SteeringState {
    int8_t angle;
    int8_t feedback;

    // TODO blinker
} shm_steering;

// Hardware setup
void steering_setup() {
    memset(&shm_steering, 0, sizeof(shm_steering));
    steering_servo_init();

    adc_init();
    adc_gpio_init(STEERING_FEEDBACK_IO);

    uwb_init();
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

static uint8_t rx_buffer[127];
static uint8_t tx_msg[125];

static void steering_uwb_task(void *arg) {
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
        shm_steering.angle += 2;
        if (shm_steering.angle >= 100) shm_steering.angle = -100;
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

    while (true) {
		vTaskDelayUntil(&tick, 200);

        taskENTER_CRITICAL();
        adc_select_input(STEERING_FEEDBACK_ADC);
        sleep_us(20);
        int x = adc_read();
        taskEXIT_CRITICAL();

        printf("ADC %d\n", x);
    }
}
