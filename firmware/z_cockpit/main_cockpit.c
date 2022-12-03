#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include <stdio.h>
#include <string.h>
#include "peripherals/testpoints.h"
#include "peripherals/steering_wheel.h"

#include "decawave/uwb_config.h"
#include "decawave/uwb_library.h"
#include "config.h"
#include "utils.h"
#include "types.h"

struct CockpitState {
    int8_t angle;
    uint8_t throttle;
    bool btn_brake;
    bool btn_BL;
    bool btn_BR;

    int8_t feedback;
    float voltage_vbat;
    float voltage_vreg;

    bool blinker_left;
    bool blinker_right;
} shm_cockpit;

// Hardware setup
void cockpit_setup() {
    memset(&shm_cockpit, 0, sizeof(shm_cockpit));
    wheel_setup();

    uwb_init();
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


static uint8_t tx_pkt_raw[120];
static uint8_t rx_pkt_raw[128];

static void cockpit_uwb_task(void *arg) {
	TickType_t tick = xTaskGetTickCount();

    int ctr = 0;
    bool success;

    int num_steering_failures = 0;
    int num_drivetrain_failures = 0;
    bool is_failure_state = false;

    // Use fixed-size buffers to facilitate encryption,
    // but alias them to structs to make manipulation easier
    cockpit_pkt_t *tx_pkt = (void*) &tx_pkt_raw;
    steering_pkt_t *pkt_s = (void*) &rx_pkt_raw;
    drivetrain_pkt_t *pkt_d = (void*) &rx_pkt_raw;

    while (true) {
        // Populate the packet based on current state
        tx_pkt->src = ZONE_ID;
        tx_pkt->angle = shm_cockpit.angle;
        tx_pkt->throttle = shm_cockpit.throttle;
        tx_pkt->brake = shm_cockpit.btn_brake;
        tx_pkt->is_failure_state = is_failure_state;
        tx_pkt->blinker_start_ts = 0;
        tx_pkt->blink_left = false;
        tx_pkt->blink_right = false;

        // Request-response with steering zone
        tx_pkt->dst = ZID_STEERING;
        tx_pkt->current_ts = 0; // TODO millis();
        // TODO encryption
        send_msg(sizeof(tx_pkt_raw), tx_pkt_raw, 0);
        memset(rx_pkt_raw, 0, sizeof(rx_pkt_raw));
        success = (receive_msg(rx_pkt_raw) != -1);
        success = success && (pkt_d->src == ZID_STEERING) &&
                    (pkt_d->dst == ZONE_ID);
        // TODO decryption
        if (success) {
            num_steering_failures = 0;
            shm_cockpit.feedback = pkt_s->feedback;
        }
        else num_steering_failures++;

        busy_wait_us(2000);


        // Request-response with drivetrain zone
        tx_pkt->dst = ZID_DRIVETRAIN;
        tx_pkt->current_ts = 0; // TODO millis();
        // TODO encryption
        send_msg(sizeof(tx_pkt_raw), tx_pkt_raw, 0);
        memset(rx_pkt_raw, 0, sizeof(rx_pkt_raw));
        success = (receive_msg(rx_pkt_raw) != -1);
        success = success && (pkt_d->src == ZID_DRIVETRAIN) &&
                    (pkt_d->dst == ZONE_ID);
        // TODO decryption
        if (success) {
            num_drivetrain_failures = 0;
            shm_cockpit.voltage_vbat = pkt_d->voltage_vbat;
            shm_cockpit.voltage_vreg = pkt_d->voltage_vreg;
        }
        else num_drivetrain_failures++;

        busy_wait_us(2000);


        if ((num_steering_failures > MAX_COMM_FAILURES) || 
                (num_drivetrain_failures > MAX_COMM_FAILURES)) {

            is_failure_state = true;

            // Don't inundate console with prints
            if ((ctr % 10) == 0) {
                printf("failure state %d %d\n", num_steering_failures,
                        num_drivetrain_failures);
            }

        } else {
            is_failure_state = false;
        }
        tp_statusled(is_failure_state);

        ctr++;
    }
}

static void cockpit_wheel_read_task(void *arg) {
	TickType_t tick = xTaskGetTickCount();
    
    float iir = 0;

    while (true) {
		vTaskDelayUntil(&tick, 20);
        wheel_update();

        int angle = wheel_get_angle();
        iir = 0.8*iir + 0.2*angle;
        iir = clamp(iir, -100, 100);
        

        shm_cockpit.angle = clamp(iir, -95, 95);
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
        //shm_cockpit.feedback = shm_cockpit.btn_brake ? 0 : shm_cockpit.angle;
		vTaskDelayUntil(&tick, 20);
    }
}
