#include <stdint.h>
#include "blinker_fsm.h"
#include "config.h"

enum blinker_states {
    NO_BLINK,
    RIGHT_BLINK,
    RIGHT_BLINK_2,
    LEFT_BLINK,
    LEFT_BLINK_2,
} blinker_state;

bool btn_left_last, btn_right_last;

int blinker_fsm_transition(bool btn_left, bool btn_right, int8_t angle) {
    switch(blinker_state) {
        case NO_BLINK: {
            if (btn_left && !btn_left_last) blinker_state = LEFT_BLINK;
            if (btn_right && !btn_right_last) blinker_state = RIGHT_BLINK;
            break;
        }

        case LEFT_BLINK: {
            if (btn_left && !btn_left_last) blinker_state = NO_BLINK;
            if (btn_right && !btn_right_last) blinker_state = RIGHT_BLINK;
            if (angle < -STEER_THRESH2) blinker_state = LEFT_BLINK_2;
            break;
        }

        case RIGHT_BLINK: {
            if (btn_left && !btn_left_last) blinker_state = LEFT_BLINK;
            if (btn_right && !btn_right_last) blinker_state = NO_BLINK;
            if (angle > STEER_THRESH2) blinker_state = RIGHT_BLINK_2;
            break;
        }

        case LEFT_BLINK_2: {
            if (btn_left && !btn_left_last) blinker_state = NO_BLINK;
            if (btn_right && !btn_right_last) blinker_state = RIGHT_BLINK;
            if (angle > -STEER_THRESH1) blinker_state = NO_BLINK;
            break;
        }

        case RIGHT_BLINK_2: {
            if (btn_left && !btn_left_last) blinker_state = LEFT_BLINK;
            if (btn_right && !btn_right_last) blinker_state = NO_BLINK;
            if (angle < STEER_THRESH1) blinker_state = NO_BLINK;
            break;
        }
    }

    btn_left_last = btn_left;
    btn_right_last = btn_right;
    
    switch (blinker_state) {
        case LEFT_BLINK: return BLINKER_LEFT;
        case LEFT_BLINK_2: return BLINKER_LEFT;
        case RIGHT_BLINK: return BLINKER_RIGHT;
        case RIGHT_BLINK_2: return BLINKER_RIGHT;
        default: return BLINKER_NONE;
    }

    return BLINKER_NONE;
}
