#include <stdint.h>

/* Motor initialization */
void drivetrain_motor_init();

/* Speed and Brake control */
void drivetrain_motor_set(int8_t motor_speed, bool brake_state); 
