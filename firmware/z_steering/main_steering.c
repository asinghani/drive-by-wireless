#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include <stdio.h>
#include <string.h>
#include "peripherals/testpoints.h"
#include "peripherals/steering_servo.h"
#include "peripherals/blinkers.h"
#include "peripherals/led_strip.h"

#include "pico/stdlib.h"
#include "pico/stdio.h"
#include "hardware/adc.h"
#include "hardware/watchdog.h"
#include "config.h"

#include "crypto/crypto.h"
#include "decawave/uwb_config.h"
#include "decawave/uwb_library.h"
#include "config.h"
#include "utils.h"
#include "types.h"

struct SteeringState {
    int8_t angle;
    int8_t feedback;

    // ts_offset = (our clock) - (base clock)
    int32_t ts_offset;
    int32_t blinker_basis_ts;
    bool blink_left;
    bool blink_right;

    bool is_failure_state;
} shm_steering;

// Hardware setup
void steering_setup() {
    memset(&shm_steering, 0, sizeof(shm_steering));
    steering_servo_init();

    uwb_init();
    blinkers_init();
    led_strip_init();
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

void _steering_uwb_task_idle() {
    blinkers_update(shm_steering.ts_offset, shm_steering.blinker_basis_ts, shm_steering.blink_left,
                        shm_steering.blink_right, shm_steering.is_failure_state);
}

static uint8_t tx_pkt_raw[120];
static uint8_t tx_pkt_enc[120];
static uint8_t rx_pkt_enc[128];
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
        memset(rx_pkt_enc, 0, sizeof(rx_pkt_enc));
        success = (receive_msg(rx_pkt_enc, _steering_uwb_task_idle) != -1);
        success = success && crypto_decrypt(rx_pkt_enc, rx_pkt_raw);
        if (success && (rx_pkt->src == ZID_COCKPIT) && 
                (rx_pkt->dst == ZONE_ID)) {
            tp_raise(ZS_TP_UWB_RX);

            last_rx_ts = millis();
            shm_steering.angle = rx_pkt->angle;
            shm_steering.ts_offset = last_rx_ts - rx_pkt->current_ts;
            shm_steering.blinker_basis_ts = rx_pkt->blinker_basis_ts;
            shm_steering.blink_left = rx_pkt->blink_left;
            shm_steering.blink_right = rx_pkt->blink_right;
            shm_steering.is_failure_state = rx_pkt->is_failure_state;

            tx_pkt->src = ZONE_ID;
            tx_pkt->dst = ZID_COCKPIT;
            tx_pkt->feedback = shm_steering.feedback;

            crypto_encrypt(tx_pkt_raw, tx_pkt_enc);
            tp_raise(ZD_TP_UWB_TX);
            send_msg(sizeof(tx_pkt_enc), tx_pkt_enc, 0, _steering_uwb_task_idle);
        }

        if ((millis() - last_rx_ts) > COMM_TIMEOUT_MS) {
            shm_steering.is_failure_state = true;
            shm_steering.angle = 0;
        }

        _steering_uwb_task_idle();
        watchdog_update();
        tp_statusled(shm_steering.is_failure_state);
        ctr++;
    }
}

static void steering_servo_task(void *arg) {
	TickType_t tick = xTaskGetTickCount();

    while (true) {
		vTaskDelayUntil(&tick, 10);
        steering_servo_set(shm_steering.angle);
        tp_raise(ZS_TP_STEER_SET);
    }
}

static void steering_feedback_task(void *arg) {
	TickType_t tick = xTaskGetTickCount();

    while (true) {
		vTaskDelayUntil(&tick, 10);
        shm_steering.feedback = steering_servo_get_feedback(shm_steering.angle);
        tp_raise(ZS_TP_FORCE_READ);
        led_strip_set();
    }
}
