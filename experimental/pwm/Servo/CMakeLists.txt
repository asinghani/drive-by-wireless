add_executable(pwm_servo
        pwm_servo.c
        )

# pull in common dependencies
target_link_libraries(pwm_servo pico_stdlib)
target_link_libraries(pwm_servo hardware_pwm)
target_link_libraries(pwm_servo hardware_clocks)

# create map/bin/hex file etc.
pico_add_extra_outputs(pwm_servo)

# add url via pico_set_program_url
example_auto_set_url(pwm_servo)
