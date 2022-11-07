cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(test)
pico_sdk_init()

FILE(GLOB sources "*/*.c" "*.c")
FILE(GLOB headers "*/*.h" "*.h")

add_executable(test
    ${sources}
    ${headers}
)

# Pico library
target_link_libraries(test
    pico_stdlib
    pico_multicore
    hardware_i2c
    hardware_spi
)

pico_enable_stdio_usb(test 1)
pico_enable_stdio_uart(test 0)

# Create programming files
pico_add_extra_outputs(test)
