# Distributed Embedded Final Project

UWB drive-by-wireless

## Build Instructions

1. Clone pico-sdk and pico-extras and put them in your environment variables as `PICO_SDK_PATH` and `PICO_EXTRAS_PATH`. Also clone FreeRTOS-Kernel, `smp` branch and put its path as `FREERTOS_KERNEL_PATH`

2. `cd firmware`

3. `rm -r build; mkdir build; cd build` (its safest to delete and re-make the build directory every time because CMake caches things incorrectly sometimes)

4. `cmake .. && make -j8` (this will build all 3 targets - cockpit, steering, and drivetrain)

5. This should yield `zone_cockpit.uf2`, `zone_steering.uf2`, and `zone_drivetrain.uf2` which can be flashed to the boards.

## File Structure

```
- firmware/ (the directory for the firmware)
    - main.c (main entry point, shouldn't put any more code into this file)
    - utils.h (utility functions for math, safety, etc.)
    - config.h (configuration of GPIO pins, etc.)
    - FreeRTOSConfig.h
    - z_cockpit/
        - main_cockpit.c (entry point for cockpit zone)
    - z_drivetrain/
        - main_drivetrain.c (entry point for drivetrain zone)
    - z_steering/
        - main_steering.c (entry point for steering zone)
    - common/ (common stuff)
        - freertos_callbacks.c
    - peripherals/ (drivers for hardware peripherals)
        - testpoints.h (driver for test points)

- experimental/ (for test programs)

- scripts/ (scripts to run on a host machine)
    - proxy_gui.py (the steering wheel proxy, to run on Windows)

- hardware/ (hardware / CAD files)
    - 449pcb_v1/ (PCB CAD)
    - schematic.pdf (board schematic, for easy reference)

```

