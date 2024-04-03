cmake_minimum_required(VERSION 3.28)

include(pico_sdk_import.cmake)

set(WIFI_SSID "${WIFI_SSID}" CACHE INTERNAL "WiFi SSID for examples")
set(WIFI_PASSWORD "${WIFI_PASSWORD}" CACHE INTERNAL "WiFi password for examples")

set(PICO_BOARD "pico_w")

include(pico_extras_import_optional.cmake)

project(rpi_pico_display_demo C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the SDK
pico_sdk_init()

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
)

if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wno-maybe-uninitialized)
endif()

# Hardware-specific examples in subdirectories:
add_executable(rpi_pico_display_demo picow_blink.c)

target_link_libraries(rpi_pico_display_demo
        pico_stdlib              # for core functionality
        pico_cyw43_arch_none     # we need Wifi to access the GPIO, but we don't need anything else
)

# create map/bin/hex file etc.
pico_add_extra_outputs(rpi_pico_display_demo)

