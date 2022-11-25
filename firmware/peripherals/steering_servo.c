#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

#include "config.h"
#include "utils.h"
#include "steering_servo.h"

float servo_clock_div = 0;
float servo_wrap = 0;

void steering_servo_init() {
    gpio_set_function(STEERING_SERVO_IO, GPIO_FUNC_PWM);
    int slice_num = pwm_gpio_to_slice_num(STEERING_SERVO_IO);
    pwm_config config = pwm_get_default_config();

    uint64_t clock_speed = clock_get_hz(clk_sys);
    servo_clock_div = 64;
    while (clock_speed/servo_clock_div/50 > 65535 && servo_clock_div < 256) servo_clock_div += 64; 
    servo_wrap = clock_speed/servo_clock_div/50;

    pwm_config_set_clkdiv(&config, servo_clock_div);
    pwm_config_set_wrap(&config, servo_wrap);
    pwm_init(slice_num, &config, true);
    pwm_set_gpio_level(STEERING_SERVO_IO, (STEERING_SERVO_CENTER/20000.f)*servo_wrap);
}

void steering_servo_set(int8_t angle) {
    int x = clamp(angle, -100, 100);
    x = (x * STEERING_SERVO_RANGE) / 100;
    x = x + STEERING_SERVO_CENTER;

    taskENTER_CRITICAL();
    pwm_set_gpio_level(STEERING_SERVO_IO, (x/20000.f)*servo_wrap);
    taskEXIT_CRITICAL();
}
