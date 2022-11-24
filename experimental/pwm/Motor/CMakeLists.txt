add_executable(pwm_motor
        pwm_motor.c
        )

# pull in common dependencies
target_link_libraries(pwm_motor pico_stdlib)
target_link_libraries(pwm_motor hardware_gpio)
target_link_libraries(pwm_motor hardware_pwm)
target_link_libraries(pwm_motor pico_binary_info)

# create map/bin/hex file etc.
pico_add_extra_outputs(pwm_motor)

# add url via pico_set_program_url
example_auto_set_url(pwm_motor)
