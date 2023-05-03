cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(jsd C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(joystick_display
        joystick_display.c
        )

target_link_libraries(joystick_display pico_stdlib hardware_adc)

# create map/bin/hex file etc.
pico_add_extra_outputs(joystick_display)

pico_enable_stdio_usb(joystick_display 1)
pico_enable_stdio_uart(joystick_display 0)

# add url via pico_set_program_url
example_auto_set_url(joystick_display)
