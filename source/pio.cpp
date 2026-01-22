#include "z80_io_sampling.pio.h"
#include "z80_in_data.pio.h"

// io_sampling 初期化
// base_pin = 0
void z80_io_sampling_program_init(PIO pio, uint sm, uint offset, uint base_pin)
{
	pio_sm_config c = z80_io_sampling_program_get_default_config(offset);
	// GPIO0–18 を IN 用 pins に設定
	sm_config_set_in_pins(&c, base_pin);
	// SHIFT 設定 (右詰め autopush OFF)
	sm_config_set_in_shift(&c, false, false, 32);
	// FIFO を RX のみに使用
	sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_RX);
	// 4分周
	sm_config_set_clkdiv(&c, 4.0f);
	// GPIO0–18 を PIO 管轄に
	for (int i = base_pin; i < base_pin + 19; ++ i)
	{
		pio_gpio_init(pio, i);
	}
	// 全ピン input
	pio_sm_set_consecutive_pindirs(pio, sm, base_pin, 19, false);
	// state machine 初期化
	pio_sm_init(pio, sm, offset, &c);
}

// in_data
// data_pin_base = 11
void z80_in_data_program_init(PIO pio, uint sm, uint offset, uint data_pin_base)
{
	pio_sm_config c = z80_in_data_program_get_default_config(offset);
	// OUT 用ピン (D0–D7)
	sm_config_set_out_pins(&c, data_pin_base, 8);
	// OUT SHIFT 設定 (LSB → D0)
	sm_config_set_out_shift(&c, false, false, 32);
	// FIFO は TX のみ使用
	sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX);
	// 4分周
	sm_config_set_clkdiv(&c, 4.0f);
	// state machine 初期化
	pio_sm_init(pio, sm, offset, &c);
}
