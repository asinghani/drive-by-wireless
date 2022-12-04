#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

#include "config.h"
#include "utils.h"
#include "drive_motor.h"

int drive_pwm_slice;

void drive_motor_init() {
    gpio_init(DRIVETRAIN_IN1);
    gpio_set_dir(DRIVETRAIN_IN1, GPIO_OUT);
    gpio_init(DRIVETRAIN_IN2);
    gpio_set_dir(DRIVETRAIN_IN2, GPIO_OUT);

    gpio_set_function(DRIVETRAIN_EN, GPIO_FUNC_PWM);
    gpio_set_dir(DRIVETRAIN_EN, GPIO_OUT);

    drive_pwm_slice = pwm_gpio_to_slice_num(DRIVETRAIN_EN);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 256);
    pwm_init(drive_pwm_slice, &config, true);
    pwm_set_wrap(drive_pwm_slice, 5000);
}

void drive_motor_set(int8_t speed, bool brake) {
    // Attempt to linearize the speed to some sensible range
    int x = clamp(speed, 0, 100);
    if (x > 0) x = 25 + (12*x)/16;
    x = clamp(x, 0, 100);
    if (brake) x = 0;
    if (x < 5) x = 0;

    taskENTER_CRITICAL();

    if (brake) {
        pwm_set_enabled(drive_pwm_slice, false);
        gpio_set_function(DRIVETRAIN_EN, GPIO_FUNC_SIO);
        gpio_set_dir(DRIVETRAIN_EN, GPIO_OUT);
        gpio_put(DRIVETRAIN_EN, 1);
        gpio_put(DRIVETRAIN_IN1, 0);
        gpio_put(DRIVETRAIN_IN2, 0);

    } else if (speed == 0) {
        pwm_set_enabled(drive_pwm_slice, false);
        gpio_set_function(DRIVETRAIN_EN, GPIO_FUNC_SIO);
        gpio_set_dir(DRIVETRAIN_EN, GPIO_OUT);
        gpio_put(DRIVETRAIN_EN, 0);
        gpio_put(DRIVETRAIN_IN1, 0);
        gpio_put(DRIVETRAIN_IN2, 0);

    } else {
        gpio_put(DRIVETRAIN_IN1, 0);
        gpio_put(DRIVETRAIN_IN2, 1);
        gpio_set_function(DRIVETRAIN_EN, GPIO_FUNC_PWM);
        pwm_set_enabled(drive_pwm_slice, true);
        pwm_set_wrap(drive_pwm_slice, 5000);
        pwm_set_gpio_level(DRIVETRAIN_EN, 48*clamp(x, 0, 100));
    }

    taskEXIT_CRITICAL();
}
