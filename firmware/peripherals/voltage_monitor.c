#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

#include "config.h"
#include "utils.h"
#include "voltage_monitor.h"

void voltage_monitor_init() {
    // Set up the current-sensor for feedback
    adc_init();
    adc_gpio_init(VMON_VBAT);
    adc_gpio_init(VMON_5V);
}

float voltage_monitor_get_vbat() {
    taskENTER_CRITICAL();
    adc_select_input(VMON_VBAT_ADC);
    busy_wait_us(20);
    float adc_out = adc_read();
    taskEXIT_CRITICAL();
    return adc_out*VMON_VBAT_SCALE;
}

float voltage_monitor_get_5v() {
    taskENTER_CRITICAL();
    adc_select_input(VMON_5V_ADC);
    busy_wait_us(20);
    float adc_out = adc_read();
    taskEXIT_CRITICAL();
    return adc_out*VMON_5V_SCALE;
}
