cmake_minimum_required(VERSION 3.24)
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

if(WIN32)
    MESSAGE(STATUS "Windows host detected. C:/SysGCC supposed to be tools root")
    ##
    ## Toolchain base definitions for SysGCC
    ##
    set(TOOLCHAIN_DIR C:/SysGCC/raspberry64)
    list(APPEND CMAKE_PROGRAM_PATH "${TOOLCHAIN_DIR}/bin/")

    ##
    ## Sysroot for SysGCC
    ##
    set(CMAKE_SYSROOT ${TOOLCHAIN_DIR}/aarch64-linux-gnu/sysroot)
    set(CMAKE_SYSROOT_LINK ${CMAKE_SYSROOT})
endif ()

##
## Toolchain settings
##
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)
set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
find_program(OBJCOPY_PROGRAM aarch64-linux-gnu-objcopy)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

##
## Project definition
##

project(rpi-led-shim C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXECUTABLE_SUFFIX .elf)


##
## Project code itself
##
#include_directories(rpi-libs pimoroni-led-shim)
include_directories(pimoroni-led-shim)
add_executable(${PROJECT_NAME} main.cpp pimoroni-led-shim/pimoroni-led-shim.c pimoroni-led-shim/i2c.c)
target_link_libraries(${PROJECT_NAME} )

##
## Generate stripped .elf
##
add_custom_target(${PROJECT_NAME}-stripped ALL
        ${OBJCOPY_PROGRAM} -g $<TARGET_FILE:${PROJECT_NAME}> ${PROJECT_NAME}-stripped${CMAKE_EXECUTABLE_SUFFIX}
        DEPENDS ${PROJECT_NAME}
        BYPRODUCTS ${PROJECT_NAME}-stripped${CMAKE_EXECUTABLE_SUFFIX}
)

##
## Write .gdbinit file
##
file(GENERATE OUTPUT ${CMAKE_SOURCE_DIR}/.gdbinit CONTENT "define target hookpost-remote\n\tsymbol-file $<TARGET_FILE:${PROJECT_NAME}>\n\tinfo files\nend\n")