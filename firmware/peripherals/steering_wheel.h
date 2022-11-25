#include <stdint.h>
#include <stdbool.h>

void wheel_setup();
bool wheel_update();
void wheel_send_feedback(int8_t feedback);
int8_t wheel_get_angle();
uint8_t wheel_get_throttle();
bool wheel_get_BL();
bool wheel_get_BR();
bool wheel_get_A();
bool wheel_get_B();
bool wheel_get_X();
bool wheel_get_Y();
bool wheel_get_brake();

