#include "steering_wheel.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include <stdio.h>

#define STEERING_UART uart1

int8_t steering = 0;
uint8_t throttle = 0;
uint8_t buttons = 0;

void steering_setup() {
    uart_init(STEERING_UART, 115200);
    gpio_set_function(20, GPIO_FUNC_UART);
    gpio_set_function(21, GPIO_FUNC_UART);

    while (uart_is_readable(STEERING_UART)) uart_getc(STEERING_UART);
}

void steering_update() {
    while (true) {
        if (!uart_is_readable(STEERING_UART)) break;

        // Try to find a start byte
        uint8_t x;
        while (uart_is_readable_within_us(STEERING_UART, 1000)) {
            x = uart_getc(STEERING_UART);
            if (x == 1) break;
        }
        if (x != 1) break;

        if (!uart_is_readable_within_us(STEERING_UART, 2000)) break;
        uint8_t m_steering = uart_getc(STEERING_UART);

        if (!uart_is_readable_within_us(STEERING_UART, 2000)) break;
        uint8_t m_throttle = uart_getc(STEERING_UART);

        if (!uart_is_readable_within_us(STEERING_UART, 2000)) break;
        uint8_t m_buttons = uart_getc(STEERING_UART);

        if (!uart_is_readable_within_us(STEERING_UART, 2000)) break;
        uint8_t m_chk = uart_getc(STEERING_UART);

        // Checksum
        if (m_chk != ((uint8_t)(m_steering + m_throttle + m_buttons))) break;

        steering = (int8_t) m_steering;
        throttle = m_throttle;
        buttons = m_buttons;
        printf("got packet %d %d %x\n", (int8_t)m_steering, m_throttle, m_buttons);
    }
}

void steering_send_feedback(int8_t feedback) {
    printf("feedback %d\n", feedback);
    uart_putc(STEERING_UART, feedback);
}

int8_t steering_get_angle() { return steering; }
uint8_t steering_get_throttle() { return throttle; }
bool steering_get_BL() { return !!(buttons & 0x80); }
bool steering_get_BR() { return !!(buttons & 0x40); }
bool steering_get_A() { return !!(buttons & 0x20); }
bool steering_get_B() { return !!(buttons & 0x10); }
bool steering_get_X() { return !!(buttons & 0x08); }
bool steering_get_Y() { return !!(buttons & 0x04); }
bool steering_get_brake() { return !!(buttons & 0x02); }

