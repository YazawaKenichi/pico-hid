cmake_minimum_required(VERSION 3.13)

add_executable(board_test)

target_sources(board_test PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/main.c
        ${CMAKE_CURRENT_LIST_DIR}/led.c
        ${CMAKE_CURRENT_LIST_DIR}/switch.c
        ${CMAKE_CURRENT_LIST_DIR}/volume.c
        ${CMAKE_CURRENT_LIST_DIR}/common.c
        ${CMAKE_CURRENT_LIST_DIR}/mode.c
        ${CMAKE_CURRENT_LIST_DIR}/board_time.c
        )

# Make sure TinyUSB can find tusb_config.h
target_include_directories(board_test PUBLIC
        ${CMAKE_CURRENT_LIST_DIR})

# In addition to pico_stdlib required for common PicoSDK functionality, add dependency on tinyusb_device
# for TinyUSB device support and tinyusb_board for the additional board support library used by the example
target_link_libraries(board_test PUBLIC pico_stdlib pico_time hardware_adc)

# Uncomment this line to enable fix for Errata RP2040-E5 (the fix requires use of GPIO 15)
#target_compile_definitions(foot_key PUBLIC PICO_RP2040_USB_DEVICE_ENUMERATION_FIX=1)

pico_add_extra_outputs(board_test)

# add url via pico_set_program_url
example_auto_set_url(board_test)

