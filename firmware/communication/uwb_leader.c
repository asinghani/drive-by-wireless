#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"

#include "decawave/deca_device_api.h"
#include "decawave/deca_regs.h"
#include "decawave/uwb_config.h"
#include "decawave/uwb_library.h"
#include "uwb_leader.h"
#include "config.h"
#include "utils.h"

// Buffer to store number of times where other zones are missed
static uint8_t zone_msg[2];

static uint8_t rx_buffer[127];
static uint8_t tx_msg[125];

// These are the states for the Cockpit Zone
typedef enum State {Communicating = 1, Waiting = 0} state_t;

void uwb_leader_init() {
    uwb_init();
}

void uwb_leader_loop() {



    while(1);

    state_t currState = Communicating;
    absolute_time_t startTime, currTime;
    startTime = get_absolute_time();
    tx_msg[0] = ZONE_ID;

    // Zero missed messages
    zone_msg[0] = 0;
    zone_msg[1] = 0;

    while (true) {

        // Execute
        switch(currState) {
            case Communicating:

                // Timestamp message
                currTime = get_absolute_time();
                *(absolute_time_t *)(tx_msg + TIME_INDEX) = currTime;

                // Broadcast Message
                send_msg(sizeof(tx_msg), tx_msg, 0);

                // Sleep to wait for next window
                sleep_us(4000);
                break;

            case Waiting:
                if (receive_msg(rx_buffer) != -1) {
                    // Reset missing if obtained any messages from zone
                    printf("got %d\n", rx_buffer[SOURCE_ID]);
                    if (rx_buffer[SOURCE_ID] == 1) zone_msg[0] = 0;
                    if (rx_buffer[SOURCE_ID] == 2) zone_msg[1] = 0;
                }
                break;
        }

        // Check if time to switch states
        switch(currState) {
            case Communicating:
                // Since we only ever need to send one message, once we're done
                // with the message we just go directly to Waiting state
                printf("Waiting\n");
                currState = Waiting;
                break;

            case Waiting:
                currTime = get_absolute_time();
                if (absolute_time_diff_us(startTime, currTime) > (3 * PERIOD)) {
                    currState = Communicating;
                    startTime = currTime;
                    printf("New Cycle! %u\n", startTime);
                    // Increase missed messages (cap to prevent overflow)
                    if (zone_msg[0] < 10) zone_msg[0] += 1;
                    if (zone_msg[1] < 10) zone_msg[1] += 1;
                }
                break;
        }

        if (zone_msg[0] > MAX_MISS) {
            printf("no z1\n");
        }
        if (zone_msg[1] > MAX_MISS) {
            printf("no z2\n");
        }
    }

    xassert(!"should never end UWB loop");
}
