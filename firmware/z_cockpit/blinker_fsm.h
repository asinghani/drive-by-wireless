#ifndef BLINKER_FSM
#define BLINKER_FSM

#include <stdint.h>
#include <stdbool.h>

#define BLINKER_NONE  0
#define BLINKER_LEFT  1
#define BLINKER_RIGHT 2

int blinker_fsm_transition(bool btn_left, bool btn_right, int8_t angle);

#endif
