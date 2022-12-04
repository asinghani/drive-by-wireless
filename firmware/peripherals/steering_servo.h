#include <stdint.h>

void steering_servo_init();
void steering_servo_set(int8_t angle); // -100 to 100
int8_t steering_servo_get_feedback(int8_t cur_angle);
