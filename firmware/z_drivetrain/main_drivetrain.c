#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include <stdio.h>
#include <string.h>
#include "peripherals/testpoints.h"
#include "peripherals/blinkers.h"
#include "peripherals/drive_motor.h"
#include "peripherals/voltage_monitor.h"

#include "pico/stdlib.h"
#include "pico/stdio.h"
#include "hardware/adc.h"
#include "hardware/uart.h"
#include "hardware/watchdog.h"
#include "config.h"

#include "crypto/crypto.h"
#include "decawave/uwb_config.h"
#include "decawave/uwb_library.h"
#include "config.h"
#include "utils.h"
#include "types.h"

struct DrivetrainState {
    uint8_t throttle;
    bool brake;
    float voltage_vbat;
    float voltage_vreg;

    // ts_offset = (our clock) - (base clock)
    int32_t ts_offset;
    int32_t blinker_basis_ts;
    bool blink_left;
    bool blink_right;

    bool is_failure_state;
} shm_drivetrain;

// Hardware setup
void drivetrain_setup() {
    memset(&shm_drivetrain, 0, sizeof(shm_drivetrain));

    uwb_init();
    blinkers_init();
    drive_motor_init();
    voltage_monitor_init();

    // Setup LED strip
    uart_init(DRIVETRAIN_LED_STRIP_UART, 115200);
    gpio_set_function(DRIVETRAIN_LED_STRIP_TX, GPIO_FUNC_UART);
}

static void drivetrain_uwb_task(void *arg);
static void drivetrain_motor_task(void *arg);
static void drivetrain_vmon_task(void *arg);
static void drivetrain_led_strip_task(void *arg);

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

    TaskHandle_t vmon_task;
    xTaskCreate(drivetrain_vmon_task, "drivetrain_vmon_task",
            configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 4, &vmon_task);
    vTaskCoreAffinitySet(vmon_task, 1);

    TaskHandle_t led_strip_task;
    xTaskCreate(drivetrain_led_strip_task, "drivetrain_led_strip_task",
            configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 5, &led_strip_task);
    vTaskCoreAffinitySet(led_strip_task, 1);

    printf("Starting scheduler\n");
    vTaskStartScheduler();
}

void _drivetrain_uwb_task_idle() {
    blinkers_update(shm_drivetrain.ts_offset, shm_drivetrain.blinker_basis_ts, shm_drivetrain.blink_left,
                        shm_drivetrain.blink_right, shm_drivetrain.is_failure_state);
}

static uint8_t tx_pkt_raw[120];
static uint8_t tx_pkt_enc[120];
static uint8_t rx_pkt_enc[128];
static uint8_t rx_pkt_raw[128];

static void drivetrain_uwb_task(void *arg) {
	TickType_t tick = xTaskGetTickCount();

    int ctr = 0;
    bool success;
    int32_t last_rx_ts = 0;

    // Use fixed-size buffers to facilitate encryption,
    // but alias them to structs to make manipulation easier
    drivetrain_pkt_t *tx_pkt = (void*) &tx_pkt_raw;
    cockpit_pkt_t *rx_pkt = (void*) &rx_pkt_raw;

    while (true) {
        memset(rx_pkt_raw, 0, sizeof(rx_pkt_enc));
        success = (receive_msg(rx_pkt_enc, _drivetrain_uwb_task_idle) != -1);
        success = success && crypto_decrypt(rx_pkt_enc, rx_pkt_raw);
        if (success && (rx_pkt->src == ZID_COCKPIT) && 
                (rx_pkt->dst == ZONE_ID)) {

            tp_raise(ZD_TP_UWB_RX);

            last_rx_ts = millis();
            shm_drivetrain.throttle = rx_pkt->throttle;
            shm_drivetrain.brake = rx_pkt->brake;
            shm_drivetrain.ts_offset = last_rx_ts - rx_pkt->current_ts;
            shm_drivetrain.blinker_basis_ts = rx_pkt->blinker_basis_ts;
            shm_drivetrain.blink_left = rx_pkt->blink_left;
            shm_drivetrain.blink_right = rx_pkt->blink_right;
            shm_drivetrain.is_failure_state = rx_pkt->is_failure_state;

            tx_pkt->src = ZONE_ID;
            tx_pkt->dst = ZID_COCKPIT;
            tx_pkt->voltage_vbat = shm_drivetrain.voltage_vbat;
            tx_pkt->voltage_vreg = shm_drivetrain.voltage_vreg;
            crypto_encrypt(tx_pkt_raw, tx_pkt_enc);
            tp_raise(ZD_TP_UWB_TX);
            send_msg(sizeof(tx_pkt_enc), tx_pkt_enc, 0, _drivetrain_uwb_task_idle);
        }

        if ((millis() - last_rx_ts) > COMM_TIMEOUT_MS) {
            shm_drivetrain.is_failure_state = true;
            shm_drivetrain.throttle = 0;
            shm_drivetrain.brake = true;
        }

        _drivetrain_uwb_task_idle();
        watchdog_update();
        tp_statusled(shm_drivetrain.is_failure_state);
        ctr++;
    }
}

static void drivetrain_motor_task(void *arg) {
	TickType_t tick = xTaskGetTickCount();

    while (true) {
		vTaskDelayUntil(&tick, 5);
        bool is_brake = (shm_drivetrain.is_failure_state ? 1 : shm_drivetrain.brake);
        drive_motor_set(shm_drivetrain.is_failure_state ? 0 : shm_drivetrain.throttle, is_brake);
        tp_raise(ZD_TP_MOTOR_SET);
        tp_set(ZD_TP_IS_BRAKE, is_brake);
    }
}

static void drivetrain_vmon_task(void *arg) {
	TickType_t tick = xTaskGetTickCount();

    while (true) {
		vTaskDelayUntil(&tick, 200);
        shm_drivetrain.voltage_vbat = voltage_monitor_get_vbat();
        shm_drivetrain.voltage_vreg = voltage_monitor_get_5v();
    }
}

static void drivetrain_led_strip_task(void *arg) {
	TickType_t tick = xTaskGetTickCount();

    while (true) {
		vTaskDelayUntil(&tick, 200);
        uint8_t dat = shm_drivetrain.throttle;
        if (shm_drivetrain.is_failure_state) dat = 0;
        if (shm_drivetrain.brake) dat = 0;

        uart_putc_raw(DRIVETRAIN_LED_STRIP_UART, dat);
    }
}

