#include "steering_wheel.h"
#include "config.h"
#include "utils.h"

#include <stdio.h>
#include "hardware/uart.h"
#include "hardware/gpio.h"

int8_t steering = 0;
uint8_t throttle = 0;
uint8_t buttons = 0;

uint32_t wheel_last_ms = 0;

void wheel_setup() {
    uart_init(STEERING_UART, 115200);
    gpio_set_function(STEERING_RX, GPIO_FUNC_UART);
    gpio_set_function(STEERING_TX, GPIO_FUNC_UART);

    while (uart_is_readable(STEERING_UART)) uart_getc(STEERING_UART);
}

bool wheel_update() {
    if (!uart_is_readable(STEERING_UART)) return false;

    // Try to find a start byte
    uint8_t x;
    while (uart_is_readable_within_us(STEERING_UART, 1000)) {
        x = uart_getc(STEERING_UART);
        if (x == 1) break;
    }
    if (x != 1) return false;

    if (!uart_is_readable_within_us(STEERING_UART, 2000)) return false;
    uint8_t m_steering = uart_getc(STEERING_UART);

    if (!uart_is_readable_within_us(STEERING_UART, 2000)) return false;
    uint8_t m_throttle = uart_getc(STEERING_UART);

    if (!uart_is_readable_within_us(STEERING_UART, 2000)) return false;
    uint8_t m_buttons = uart_getc(STEERING_UART);

    if (!uart_is_readable_within_us(STEERING_UART, 2000)) return false;
    uint8_t m_chk = uart_getc(STEERING_UART);

    // Checksum
    if (m_chk != ((uint8_t)(m_steering + m_throttle + m_buttons))) return false;

    steering = (int8_t) m_steering;
    throttle = m_throttle;
    buttons = m_buttons;
    wheel_last_ms = millis();

    return true;
}

void wheel_send_feedback(int8_t feedback) {
    uart_putc(STEERING_UART, feedback);
}

int8_t wheel_get_angle() { return steering; }
uint8_t wheel_get_throttle() { return throttle; }
bool wheel_get_BL() { return !!(buttons & 0x80); }
bool wheel_get_BR() { return !!(buttons & 0x40); }
bool wheel_get_A() { return !!(buttons & 0x20); }
bool wheel_get_B() { return !!(buttons & 0x10); }
bool wheel_get_X() { return !!(buttons & 0x08); }
bool wheel_get_Y() { return !!(buttons & 0x04); }
bool wheel_get_brake() { return !!(buttons & 0x02); }
bool wheel_is_valid() { return (millis() - wheel_last_ms) < 1000; }
