/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <stdlib.h>
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

// Test Defines
#define SIZE ((uint16_t)128)
#define MAX_SIZE ((uint16_t)256)
#define COUNT ((uint32_t)10000)

/* Buffer to store received frame. See NOTE 1 below. */
static uint8_t rx_buffer[MAX_SIZE];

/* Buffer to store sent frame. See NOTE 1 below. */
static uint8_t tx_msg[MAX_SIZE];

/* Buffer to store latencies */
static int64_t latency_array[1000];

int main() {
    stdio_init_all();

    uwb_init();

    #if 1

    uint32_t count = 0;
    int misses = 0;
    int corrupt_errors = 0;
    int64_t latency = 0;
    int64_t total_latency = 0;
    
    while (count < COUNT) {
        
        absolute_time_t timeStamp1, timeStamp4;
        uint64_t timeStamp2, timeStamp3;
        int64_t period, processing_time;

        // Send size
        ((uint16_t *)tx_msg)[0] = SIZE;

        printf("Count: %u\n", count);
        
        // Randomize sent message
        for (size_t i = 2; i < SIZE; i++) {
            tx_msg[i] = rand();
        }

        // Timestamp before sending
        timeStamp1 = get_absolute_time();

        // Send message
        send_msg(SIZE, tx_msg, 0);

        // Check if receive message
        if (receive_msg(rx_buffer, &timeStamp4) >= 0) {

            timeStamp2 = ((uint64_t *)rx_buffer)[0];
            timeStamp3 = ((uint64_t *)rx_buffer)[1];

            int corrupted_data = 0;
            
            // Check for corrupted_data
            for (size_t i = 16; i < SIZE; i++) {
                if (rx_buffer[i] != tx_msg[i]) {
                    corrupted_data = 1;
                }
            }

            if (corrupted_data == 0) {
                // Obtain 2 way latency
                period = absolute_time_diff_us(timeStamp1, timeStamp4);
                processing_time = timeStamp3 - timeStamp2;
                latency = period - processing_time;
                if (count < 1000) latency_array[count] = latency;
                total_latency += latency;
            }
            else {
                corrupt_errors++;
                printf("Currupted message!\n");
                if (count < 1000) latency_array[count] = 100000;
            }
        }
        else {
            misses++;
            printf("Error!\n");
        }

        sleep_us(1000);

        count++;
    }

    total_latency = total_latency / (int64_t)(int32_t)(COUNT - (uint32_t)misses - (uint32_t)corrupt_errors);

    while (true) {
        printf("Report: \nMisses - %d /10000\nCorrupted Errors - %d /10000\nAverage Latency - %lld us\n", misses, corrupt_errors, total_latency);
        printf("\nIndiv latencies\n");
        printf("Latency\n");
        for (size_t i = 0; i < 1000; i++) {
            printf("%lld\n", latency_array[i]);
        }
        sleep_ms(10000);
    }

    #else
    
    while (true) {
        absolute_time_t timeStamp2, timeStamp3;

        // Check if receive message
        if (receive_msg(rx_buffer, &timeStamp2) >= 0) {
            
            uint16_t incoming_size = ((uint16_t *)rx_buffer)[0];
            
            // Copy incoming message
            for (uint16_t i = 16; i < incoming_size; i++) {
                tx_msg[i] = rx_buffer[i];
            }
            
            // Store timestamp in message
            ((uint64_t *)tx_msg)[0] = to_us_since_boot(timeStamp2);

            // Get send time
            timeStamp3 = get_absolute_time();
            ((uint64_t *)tx_msg)[1] = to_us_since_boot(timeStamp3);

            // Send message
            send_msg(incoming_size, tx_msg, 0);
        }
        else {
            printf("Timeout!\n");
        }
    }

    #endif

    // Should never reach here
    return 0;
}
