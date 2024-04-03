cmake_minimum_required(VERSION 3.28)


set(PICO_BOARD "pico_w")

# Pull in PICO SDK (must be before project)
include(pico_sdk_import.cmake)

set(WIFI_SSID "${WIFI_SSID}" CACHE INTERNAL "WiFi SSID for examples")
set(WIFI_PASSWORD "${WIFI_PASSWORD}" CACHE INTERNAL "WiFi password for examples")


project(rpi_pico_display_demo C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the SDK

pico_sdk_init()

set(OUTPUT_NAME rpi_pico_display_demo)

include_directories(${PIMORONI_PICO_PATH})
list(APPEND CMAKE_MODULE_PATH ${PIMORONI_PICO_PATH})

add_subdirectory(${PIMORONI_PICO_PATH}/common ${CMAKE_CURRENT_BINARY_DIR}/pimoroni-common)
add_subdirectory(${PIMORONI_PICO_PATH}/drivers ${CMAKE_CURRENT_BINARY_DIR}/pimoroni-drivers)
add_subdirectory(${PIMORONI_PICO_PATH}/libraries ${CMAKE_CURRENT_BINARY_DIR}/pimoroni-libraries)


add_executable(${OUTPUT_NAME} src/ble-co2-display.cpp src/pimoroni-pico-display-2.cpp)

#if(CMAKE_BUILD_TYPE STREQUAL "Debug")
#    pico_enable_stdio_uart(${OUTPUT_NAME} 0)
#    pico_enable_stdio_semihosting(${OUTPUT_NAME} 1)
#else()
    pico_enable_stdio_uart(${OUTPUT_NAME} 1)
    pico_enable_stdio_semihosting(${OUTPUT_NAME} 0)
#endif()

target_include_directories(${OUTPUT_NAME} PRIVATE src)
target_compile_definitions(${OUTPUT_NAME} PRIVATE RUNNING_AS_CLIENT=1)

# Pull in pico libraries that we need
target_link_libraries(${OUTPUT_NAME}
        pico_stdlib
        pico_stdio_uart
        hardware_spi
        hardware_pwm
        hardware_dma
        hardware_uart
        pico_display_2
        st7789
        pico_graphics
        rgbled
        #        button
        #        pico_multicore

        pico_btstack_ble
        pico_btstack_cyw43
        pico_cyw43_arch_none

)

# create map/bin/hex file etc.
pico_add_extra_outputs(${OUTPUT_NAME})
