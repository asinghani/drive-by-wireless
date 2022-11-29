/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"

#include "deca_device_api.h"
#include "deca_regs.h"

// UWB Config and Functions
#include "uwb_config.h"
#include "uwb_library.h"

// TDMA Config
#include "tdma_config.h"

/* Buffer to store number of times where other zones are missed
   Note: This is increment every TDMA cycle 
   Note: Since there is only 1 other non-cockpit zone, this is just a uint8_t */
static uint8_t zone_msg;

/* Track Cockpit Messages missed */
static uint8_t cockpit_msg;

/* Buffer to store received frame. See NOTE 1 below. */
static uint8_t rx_buffer[127];

/* The frame sent in this example is an 802.15.4e standard blink. It is a 12-byte frame composed of the following fields:
 *     - byte 0: frame type (0xC5 for a blink).
 *     - byte 1: sequence number, incremented for each new frame.
 *     - byte 2 -> 9: device ID, see NOTE 1 below.
 */
static uint8_t tx_msg[125];

// These are the states for the Cockpit Zone
typedef enum State {Communicating = 1, Waiting = 0} state_t;


int main() {
    stdio_init_all();

    uwb_init();

    state_t currState = Waiting;

    absolute_time_t startTime, currTime;

    startTime = get_absolute_time();
    int64_t delay_us = 0;

    // First byte of transmission is always source
    tx_msg[SOURCE_ID] = ZONE_ID;

    // Zero missed messages
    zone_msg = 0;

    // Set initiated to 0
    bool initiated = 0;

    int count = 0;

    while (true) {
        
        // Execute
        switch(currState) {
            case Waiting:
                if (receive_msg(rx_buffer) != -1) {
                    // Reset missing if obtained any messages from zone
                    if (rx_buffer[SOURCE_ID] == (3 - ZONE_ID)) zone_msg = 0;

                    if (rx_buffer[SOURCE_ID] == 0) {
                        // We received an initiating message
                        initiated = 1;
                        
                        // Synchronize time
                        currTime = get_absolute_time();
                        startTime = *(absolute_time_t *)(rx_buffer + TIME_INDEX);
                        // Get us delay between zone 0 time and this zone's time
                        // We will always add this delay to this zone's time for
                        // synchronization purposes
                        delay_us = absolute_time_diff_us(startTime, currTime);

                        // Reset cockpit missed msg
                        cockpit_msg = 0;
                    }
                }
            
            case Communicating:
                // Broadcast Message
                send_msg(sizeof(tx_msg), tx_msg, 0);

                // Sleep to wait for next window
                sleep_us(4000);
                break;
        }

        // Check if time to switch states
        switch(currState) {
            case Waiting:
                // We only switch if we're initiated
                if (initiated) {
                    currTime = get_absolute_time();
                    // Add delay to get accurate timestamped time
                    if (absolute_time_diff_us(startTime, currTime) > (PERIOD * ZONE_ID + delay_us)) {
                        currState = Communicating;
                        printf("New Window!\n");
                        if (zone_msg < 10) zone_msg += 1;
                    }
                }
                break;
            case Communicating:
                // Since we only ever need to send one message, once we're done
                // with the message we just go directly to Waiting state
                printf("Waiting\n");
                currState = Waiting;
                // Reset initiated back to 0
                initiated = 0;
                break;
        }
        
        currTime = get_absolute_time();
        // If we seem to be missing 1 cycle, increment cockpit_msg
        if (absolute_time_diff_us(startTime, currTime) > (PERIOD * 6 + delay_us)) {
            if (cockpit_msg < 10) cockpit_msg += 1;
            // Reset startTime (Doesn't affect as we're waiting anyways)
            startTime = currTime;
            delay_us = 0;
        }

        //FIXME: Further test error states
        if (cockpit_msg > MAX_MISS - 1) {
            // printf("Error... Missing Zone 0, %d\n", cockpit_msg);
        }

        if (zone_msg > MAX_MISS) {
            // printf("Error... Missing Zone %d\n", 3 - ZONE_ID);
        }
    }

    // Should never reach here
    return 0;
}
