//BLINKER FSM


#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "pico/binary_info.h"





enum blinker_states {
    NO_BLINK,
    RIGHT_BLINK,
	RIGHT_BLNK_2,
    LEFT_BLINK,
	LEFT_BLINK_2,
    RESET,
} blinker_state;

enum events {
    LEFT_BUTTON_PRESS,
   	RIGHT_BUTTON_PRESS,
	RIGHT_STEER_OVER_THRESH_1,
	RIGHT_STEER_UNDER_THRESH_2,
	LEFT_STEER_OVER_THRESH_1,
	LEFT_STEER_UNDER_THRESH_2,
};


void blinker_state_trans(enum events event) {

    switch(state) {
        case NO_BLINK:
            switch(event) {
                case LEFT_BUTTON_PRESS:
                    state = LEFT_BLINK;
                    break;
                case RIGHT_BUTTON_PRESS:
                    state = RIGHT_BLINK;
                    break
                default:
                    state = NO_BLINK;
                    break;
            }
            break;

        case LEFT_BLINK:
            switch(event) {
                case LEFT_BUTTON_PRESS:
                    state = NO_BLINK;
                    break;
                case RIGHT_BUTTON_PRESS:
                    state = RIGHT_BLINK;
                    break;
                case LEFT_STEER_OVER_THRESH_1:
                    state = LEFT_BLINK_2;
                    break;
                case RESET:
                    state = NO_BLINK;
                    break;
                default:
                    state = LEFT_BLINK;
                    break;
            }
            break;

        case RIGHT_BLINK:
            switch(event) {
                case RIGHT_BUTTON_PRESS:
                    state = NO_BLINK;
                    break;
                case LEFT_BUTTON_PRESS:
                    state = LEFT_BLINK;
                    break;
                case RIGHT_STEER_OVER_THRESH_1:
                    state = RIGHT_BLINK_2;
                    break;
                case RESET:
                    state = NO_BLINK;
                    break;
                default:
                    state = RIGHT_BLINK;
                    break;
            }
            break;

        case LEFT_BLINK_2:
            switch(event) {
                case LEFT_BUTTON_PRESS:
                    state = NO_BLINK;
                    break;
                case LEFT_STEER_UNDER_THRESH_2:
                    state = NO_BLINK;
                    break;
                case RESET:
                    state = NO_BLINK;
                    break;
                default:
                    state = LEFT_BLINK_2;
                    break;
            }
            break;

        case RIGHT_BLINK_2:
            switch(event) {
                case RIGHT_BUTTON_PRESS:
                    state = NO_BLINK;
                    break;
                case RIGHT_STEER_UNDER_THRESH_2:
                    state = NO_BLINK;
                    break;
                case RESET:
                    state = NO_BLINK;
                    break;
                default:
                    state = RIGHT_BLINK_2;
                    break;
            }
            break;
    }
}
