#include "steering_wheel.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include <stdio.h>

int8_t steering = 0;
uint8_t throttle = 0;
uint8_t buttons = 0;

void steering_setup() {
    uart_init(uart0, 115200);
    gpio_set_function(0, GPIO_FUNC_UART);
    gpio_set_function(1, GPIO_FUNC_UART);

    while (uart_is_readable(uart0)) uart_getc(uart0);
}

void steering_update() {
    while (true) {
        if (!uart_is_readable(uart0)) break;

        // Try to find a start byte
        uint8_t x;
        while (uart_is_readable_within_us(uart0, 1000)) {
            x = uart_getc(uart0);
            if (x == 1) break;
        }
        if (x != 1) break;

        if (!uart_is_readable_within_us(uart0, 2000)) break;
        uint8_t m_steering = uart_getc(uart0);

        if (!uart_is_readable_within_us(uart0, 2000)) break;
        uint8_t m_throttle = uart_getc(uart0);

        if (!uart_is_readable_within_us(uart0, 2000)) break;
        uint8_t m_buttons = uart_getc(uart0);

        if (!uart_is_readable_within_us(uart0, 2000)) break;
        uint8_t m_chk = uart_getc(uart0);

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
    uart_putc(uart0, feedback);
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

