#include <stdint.h>
#include <stdbool.h>

void steering_setup();
bool steering_update();
void steering_send_feedback(int8_t feedback);
int8_t steering_get_angle();
uint8_t steering_get_throttle();
bool steering_get_BL();
bool steering_get_BR();
bool steering_get_A();
bool steering_get_B();
bool steering_get_X();
bool steering_get_Y();
bool steering_get_brake();

