cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD pimoroni_pico_plus2_rp2350)
# set(PICO_BOARD pico2)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
project(PicoZ-ish_project C CXX ASM)
pico_sdk_init()

add_executable(PicoZ-ish
    ./source/main.cpp
)

target_link_libraries(PicoZ-ish
    pico_stdlib
)
pico_set_program_name(PicoZ-ish "PicoZ-ish")
pico_set_program_version(PicoZ-ish "0.0.1")
pico_enable_stdio_usb(PicoZ-ish 1)
pico_enable_stdio_uart(PicoZ-ish 0)

target_include_directories(PicoZ-ish PRIVATE ${CMAKE_CURRENT_LIST_DIR}/source)

pico_add_extra_outputs(PicoZ-ish)
