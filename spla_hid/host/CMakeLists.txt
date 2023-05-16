cmake_minimum_required(VERSION 3.13)

add_executable(spla_hid_host)

# Example source
target_sources(spla_hid_host PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/hid_app.c
        ${CMAKE_CURRENT_LIST_DIR}/main.c
        ${CMAKE_CURRENT_LIST_DIR}/msc_app.c
        ${CMAKE_CURRENT_LIST_DIR}/cdc_app.c
        )

# Make sure TinyUSB can find tusb_config.h
target_include_directories(spla_hid_host PUBLIC
        ${CMAKE_CURRENT_LIST_DIR})

# In addition to pico_stdlib required for common PicoSDK functionality, add dependency on tinyusb_host
# for TinyUSB device support and tinyusb_board for the additional board support library used by the example
target_link_libraries(spla_hid_host PUBLIC pico_stdlib tinyusb_host tinyusb_board)

pico_add_extra_outputs(spla_hid_host)

# add url via pico_set_program_url
example_auto_set_url(spla_hid_host)
