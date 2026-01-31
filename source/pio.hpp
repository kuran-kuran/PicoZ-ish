#ifndef PIO_HPP
#define PIO_HPP

#include "hardware/pio.h"
void z80_io_sampling_program_init(PIO pio, uint sm, uint offset, uint base_pin);
void z80_in_data_program_init(PIO pio, uint sm, uint offset, uint data_pin_base);

#endif
