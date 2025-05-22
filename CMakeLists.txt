cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(cmake/pico_sdk_import.cmake)

include(cmake/pico_extras_import_optional.cmake)

project(pico_hid C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(PICO_EXAMPLES_PATH ${PROJECT_SOURCE_DIR})

# Initialize the SDK
pico_sdk_init()

include(cmake/example_auto_set_url.cmake)

add_compile_options(-Wall
    -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
    -Wno-unused-function # we have some for the docs that aren't called
    -Wno-maybe-uninitialized
    )

if(TARGET tinyusb_device)
    add_subdirectory(foot_key)
    add_subdirectory(mine_hid)
    add_subdirectory(poke_coin)
    add_subdirectory(poke_pack)
    if(TARGET tinyusb_host)
        add_subdirectory(spla_hid)
    else()
        message("Skipping TinyUSB host examples as TinyUSB is unavailable")
    endif()
else()
    message("Skipping TinyUSB device examples as TinyUSB is unavailable")
endif()



