#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include "pico/binary_info.h"
#include "hardware/gpio.h"

#include "config.h"
#include "utils.h"
#include "drivetrain_motor.h"

uint slice = 0;

void drivetrain_motor_init(){
    //gpio init for IN1 and IN2 pin of H-bridge
    gpio_init (DRIVETRAIN_FW);
    gpio_set_dir (DRIVETRAIN_FW, GPIO_OUT);
    gpio_init (DRIVETRAIN_BW);
    gpio_set_dir (DRIVETRAIN_BW, GPIO_OUT);
    
    //gpio pwm for ENA pin of H-bridge
    gpio_set_function (DRIVETRAIN_EN, GPIO_FUNC_PWM);

    slice = pwm_gpio_to_slice_num (DRIVETRAIN_EN);
    pwm_set_wrap (slice, 5120);
}

void drivetrain_motor_set(int8_t motor_speed, bool brake_state){
    taskENTER_CRITICAL();
    if (brake_state) {
        gpio_put(DRIVETRAIN_EN, 1);
        gpio_put(DRIVETRAIN_FW, 0);
        gpio_put(DRIVETRAIN_BW, 0);
    }
    else if (motor_speed == 0) {        //Stopping
        //gpio_put(DRIVETRAIN_FW, 1);
        //gpio_put(DRIVETRAIN_BW, 0);
        //pwm_set_gpio_level (DRIVETRAIN_EN, 0);
        //pwm_set_enabled(slice, false);
        gpio_put(DRIVETRAIN_EN, 0);
    }    
    else{
        gpio_put(DRIVETRAIN_FW, 1);
        gpio_put(DRIVETRAIN_BW, 0);
        pwm_set_enabled(slice, true);   
        //motor speed
        pwm_set_gpio_level (DRIVETRAIN_EN, (5120 * (50+(0.5*motor_speed))/100));
    }  
    taskEXIT_CRITICAL();
}