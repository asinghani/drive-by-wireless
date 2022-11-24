cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)
include(FreeRTOS_Kernel_import.cmake)

project(drive_by_wireless C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

FILE(GLOB sources "*/*.c" "*.c")
FILE(GLOB headers "*/*.h" "*.h")

function(setup_target target_name)
    add_executable(${target_name}
        ${sources}
        ${headers}
    )

    target_include_directories(${target_name} PRIVATE
        .
    )

    pico_enable_stdio_usb(${target_name} 1)
    pico_enable_stdio_uart(${target_name} 0)

    target_link_libraries(${target_name}
        pico_stdlib 
        FreeRTOS-Kernel
        FreeRTOS-Kernel-Heap1
    )

    pico_add_extra_outputs(${target_name})
endfunction()


# Should not need to change anything below this point
# Any library-additions should be in the above block
# to maintain consistency across build targets

setup_target(zone_cockpit)
target_compile_definitions(zone_cockpit PRIVATE
    ZONE_COCKPIT=1
    ZONE_NAME="Cockpit"
)

setup_target(zone_steering)
target_compile_definitions(zone_steering PRIVATE
    ZONE_STEERING=1
    ZONE_NAME="Steering"
)

setup_target(zone_drivetrain)
target_compile_definitions(zone_drivetrain PRIVATE
    ZONE_DRIVETRAIN=1
    ZONE_NAME="Drivetrain"
)


