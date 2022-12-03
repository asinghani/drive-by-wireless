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
#include "types.h"

struct SteeringState {
    int8_t angle;
    int8_t feedback;

    int32_t ts_offset;
    int32_t blinker_start_ts;
    bool blink_left;
    bool blink_right;

    bool is_failure_state;
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

static uint8_t tx_pkt_raw[120];
static uint8_t rx_pkt_raw[128];

static void steering_uwb_task(void *arg) {
	TickType_t tick = xTaskGetTickCount();

    int ctr = 0;
    bool success;
    int32_t last_rx_ts = 0;

    // Use fixed-size buffers to facilitate encryption,
    // but alias them to structs to make manipulation easier
    steering_pkt_t *tx_pkt = (void*) &tx_pkt_raw;
    cockpit_pkt_t *rx_pkt = (void*) &rx_pkt_raw;

    while (true) {
        memset(rx_pkt_raw, 0, sizeof(rx_pkt_raw));
        success = (receive_msg(rx_pkt_raw) != -1);
        if (success && (rx_pkt->src == ZID_COCKPIT) && 
                (rx_pkt->dst == ZONE_ID)) {

            printf("RX %d %d %d\n", success, rx_pkt->src, rx_pkt->dst);

            last_rx_ts = millis();
            shm_steering.angle = rx_pkt->angle;
            shm_steering.ts_offset = 0; // TODO
            shm_steering.blinker_start_ts = rx_pkt->blinker_start_ts;
            shm_steering.blink_left = rx_pkt->blink_left;
            shm_steering.blink_right = rx_pkt->blink_right;
            shm_steering.is_failure_state = rx_pkt->is_failure_state;

            tx_pkt->src = ZONE_ID;
            tx_pkt->dst = ZID_COCKPIT;
            tx_pkt->feedback = shm_steering.feedback;
            send_msg(sizeof(tx_pkt_raw), tx_pkt_raw, 0);
        }

        if ((millis() - last_rx_ts) > COMM_TIMEOUT_MS) {
            shm_steering.is_failure_state = true;
            shm_steering.angle = 0;
        }

        tp_statusled(shm_steering.is_failure_state);
        ctr++;
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
		vTaskDelayUntil(&tick, 50);

        taskENTER_CRITICAL();
        adc_select_input(STEERING_FEEDBACK_ADC);
        busy_wait_us(20);
        int adc_out = adc_read();
        taskEXIT_CRITICAL();

        float state = ((shm_steering.angle > 0) ? -1 : 1)*(adc_out-35)*0.3 + (-shm_steering.angle*0.2);
        state = state * 0.5;
        if (absf(shm_steering.angle) < 5) state = 0;
        int out = state;
        out = clamp(out, -80, 80);

        shm_steering.feedback = out;
    }
}
