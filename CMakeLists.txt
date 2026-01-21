cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD pimoroni_pico_plus2_rp2350)
# set(PICO_BOARD pico2)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)
project(PicoZ-ish_project C CXX ASM)
pico_sdk_init()

set(SDIO_LIB_ROOT "D:/project/git/no-OS-FatFS-SD-SDIO-SPI-RPi-Pico")
add_subdirectory(${SDIO_LIB_ROOT}/src ${CMAKE_CURRENT_BINARY_DIR}/sd_lib)

add_executable(PicoZ-ish
    ./source/main.cpp
    ./source/hw_config.c
    ./source/sdcard.cpp
    ./source/uart.cpp
    ./source/pio.cpp
)

pico_generate_pio_header(PicoZ-ish ${CMAKE_CURRENT_LIST_DIR}/source/z80_io_sampling.pio)
pico_generate_pio_header(PicoZ-ish ${CMAKE_CURRENT_LIST_DIR}/source/z80_in_data.pio)

target_link_libraries(PicoZ-ish
    pico_stdlib
    hardware_uart
    hardware_spi
    hardware_pio
    hardware_dma
    no-OS-FatFS-SD-SDIO-SPI-RPi-Pico
)
pico_set_program_name(PicoZ-ish "PicoZ-ish")
pico_set_program_version(PicoZ-ish "0.0.1")
pico_enable_stdio_usb(PicoZ-ish 1)
pico_enable_stdio_uart(PicoZ-ish 0)

# target_include_directories(PicoZ-ish PRIVATE ${CMAKE_CURRENT_LIST_DIR}/source)
target_include_directories(PicoZ-ish PRIVATE 
    ${CMAKE_CURRENT_LIST_DIR}
    ${SDIO_LIB_ROOT}/src
    ${SDIO_LIB_ROOT}/src/include
    ${SDIO_LIB_ROOT}/src/sd_driver
    ${SDIO_LIB_ROOT}/src/ff15/source
)

pico_add_extra_outputs(PicoZ-ish)
