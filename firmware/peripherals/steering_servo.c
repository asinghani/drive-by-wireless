#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

#include "config.h"
#include "utils.h"
#include "steering_servo.h"

float servo_clock_div = 0;
float servo_wrap = 0;

void steering_servo_init() {
    // Set up the servo GPIO and PWM peripheral
    gpio_set_function(STEERING_SERVO_IO, GPIO_FUNC_PWM);
    int slice_num = pwm_gpio_to_slice_num(STEERING_SERVO_IO);
    pwm_config config = pwm_get_default_config();
    uint64_t clock_speed = clock_get_hz(clk_sys);
    servo_clock_div = 64;
    while (clock_speed/servo_clock_div/50 > 65535 && servo_clock_div < 256) servo_clock_div += 64; 
    servo_wrap = clock_speed/servo_clock_div/50;
    pwm_set_gpio_level(STEERING_SERVO_IO, (STEERING_SERVO_CENTER/20000.f)*servo_wrap);
    pwm_config_set_clkdiv(&config, servo_clock_div);
    pwm_config_set_wrap(&config, servo_wrap);
    pwm_init(slice_num, &config, true);
    pwm_set_gpio_level(STEERING_SERVO_IO, (STEERING_SERVO_CENTER/20000.f)*servo_wrap);

    // Set up the current-sensor for feedback
    adc_init();
    adc_gpio_init(STEERING_FEEDBACK_IO);
}

void steering_servo_set(int8_t angle) {
    int x = clamp(angle, -100, 100);
    x = (x * STEERING_SERVO_RANGE) / 100;
    x = x + STEERING_SERVO_CENTER;

    taskENTER_CRITICAL();
    pwm_set_gpio_level(STEERING_SERVO_IO, (x/20000.f)*servo_wrap);
    taskEXIT_CRITICAL();
}

int8_t steering_servo_get_feedback(int8_t cur_angle) {
    taskENTER_CRITICAL();
    adc_select_input(STEERING_FEEDBACK_ADC);
    busy_wait_us(20);
    int adc_out = adc_read();
    taskEXIT_CRITICAL();

    // This calculation was determined emperically to try to get a feedback
    // function that feels somewhat-realistic
    float state = ((cur_angle > 0) ? -1 : 1)*(adc_out-35)*0.3 + (-cur_angle*0.2);
    state = state * 0.5;
    if (absf(cur_angle) < 5) state = 0;
    int8_t out = clamp((int)state, -80, 80);
    return out;
}
