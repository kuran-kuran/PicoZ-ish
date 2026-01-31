#ifndef PSRAM_HPP
#define PSRAM_HPP

#include <cstdio>
#include "pico/stdlib.h"
#include "hardware/structs/ioqspi.h"
#include "hardware/structs/qmi.h"
#include "hardware/structs/xip_ctrl.h"
#include "hardware/clocks.h"
#include "hardware/sync.h"

#define PSRAM_BASE 0x11000000
#define PSRAM_SIZE (8 * 1024 * 1024)

size_t __no_inline_not_in_flash_func(psram_init)(uint cs_pin);

#endif
