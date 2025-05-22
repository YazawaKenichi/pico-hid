cmake_minimum_required(VERSION 3.13)

add_executable(poke_pack)

target_sources(poke_pack PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/main.c
        ${CMAKE_CURRENT_LIST_DIR}/usb_descriptors.c
        )

# Make sure TinyUSB can find tusb_config.h
target_include_directories(poke_pack PUBLIC
        ${CMAKE_CURRENT_LIST_DIR})

# In addition to pico_stdlib required for common PicoSDK functionality, add dependency on tinyusb_device
# for TinyUSB device support and tinyusb_board for the additional board support library used by the example
target_link_libraries(poke_pack PUBLIC pico_stdlib tinyusb_device tinyusb_board hardware_adc)

# Uncomment this line to enable fix for Errata RP2040-E5 (the fix requires use of GPIO 15)
#target_compile_definitions(poke_pack PUBLIC PICO_RP2040_USB_DEVICE_ENUMERATION_FIX=1)

pico_add_extra_outputs(poke_pack)

pico_enable_stdio_usb(poke_pack 1)
pico_enable_stdio_uart(poke_pack 0)

# add url via pico_set_program_url
example_auto_set_url(poke_pack)
