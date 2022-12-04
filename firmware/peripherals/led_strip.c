#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h"

#include "config.h"
#include "utils.h"
#include "led_strip.h"

void led_strip_init() {
	PIO pio = pio0;
	int sm = 0;
	uint offset = pio_add_program(pio, &ws2812_program);
	ws2812_program_init(pio, sm, offset, STEERING_LED_STRIP_IO, 800000, true);
}

// Call every 10ms
void led_strip_set() {
    for (int i = 0; i < 50; i++) {
        //pio_sm_put_blocking(pio0, 0, (0x400000 << 8)); // GRB
        //busy_wait_us(50);
    }
    //taskENTER_CRITICAL();
    //taskEXIT_CRITICAL();
}
