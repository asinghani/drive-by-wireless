//Pwm motor
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "pico/binary_info.h"

typedef struct
{
    uint gpio_en;
    uint gpio_fw;
    uint gpio_bw;
    uint slice;
    uint chan;
} Motor;

void motorInit(Motor *m, uint gpio_en, uint gpio_fw, uint gpio_bw)
{
    gpio_init(gpio_fw);
    gpio_set_dir(gpio_fw, GPIO_OUT);

    gpio_init(gpio_bw);
    gpio_set_dir(gpio_bw, GPIO_OUT);

    gpio_set_function(gpio_en, GPIO_FUNC_PWM);
    m->gpio_en = gpio_en;
    m->gpio_fw = gpio_fw;
    m->gpio_bw = gpio_bw;
    m->slice = pwm_gpio_to_slice_num(gpio_en);
    m->chan = pwm_gpio_to_channel(gpio_en);
    
    pwm_set_wrap (m->slice, 5120);
}

void motorspeed(Motor *m, int s)
{
    pwm_set_gpio_level (m->gpio_en, (5120 * (s) / 100) );
    
}
void motorOn(Motor *m)
{
    pwm_set_enabled(m->slice, true);
   
}

void motorBreak(Motor *m)
{
    gpio_put(m->gpio_fw, 0);
    gpio_put(m->gpio_bw, 0);

}

void motorStop(Motor *m)
{
    pwm_set_gpio_level (m->gpio_en, 0);
    pwm_set_enabled(m->slice, false);
    gpio_put(m->gpio_en, 0);

}

void motorDir(Motor *m, int dir){
    if (dir == 0){
        gpio_put(m->gpio_fw, 1);
        gpio_put(m->gpio_bw, 0);
    }else{
        gpio_put(m->gpio_fw, 0);
        gpio_put(m->gpio_bw, 1);
    }
}

int main()
{
    Motor mot1;
    motorInit(&mot1, 22, 21, 19);
    motorDir(&mot1, 0);     //0 -> forward, 1-> backward

    for (int i= 4; i<11; i++) {
        motorDir(&mot1, 0);
        motorOn(&mot1);
        motorspeed(&mot1,10*i);
        sleep_ms(5000);
        
        motorStop(&mot1);
        sleep_ms(5000);
    }
    motorBreak(&mot1);

    return 0;

}
