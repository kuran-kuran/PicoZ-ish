#include "z80_io_address.pio.h"
#include "z80_io_data.pio.h"

// GPIO0: /IORQ
// GPIO1: /RD
// GPIO2: /WR
// GPIO3: A0
// GPIO4: A1
// GPIO5: A2
// GPIO6: A3
// GPIO7: A4
// GPIO8: A5
// GPIO9: A6
// GPIO10: A7
// GPIO11: D0
// GPIO12: D1
// GPIO13: D2
// GPIO14: D3
// GPIO15: D4
// GPIO16: D5
// GPIO17: D6
// GPIO18: D7

// io_address
// base_pin = 0
void z80_io_address_program_init(PIO pio, uint sm, uint offset, uint base_pin)
{
	pio_sm_config c = z80_io_address_program_get_default_config(offset);
	// GPIO0 を IN 用 pins の先頭に設定
	sm_config_set_in_pins(&c, base_pin);
	// SHIFT 設定 (左シフト autopush OFF)
	sm_config_set_in_shift(&c, false, false, 32);
	// FIFO を RX のみに使用
	sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_RX);
	// 4分周
	sm_config_set_clkdiv(&c, 4.0f);
	// GPIO0–10 を PIO 管轄に
	for (int i = base_pin; i < base_pin + 11; ++ i)
	{
		pio_gpio_init(pio, i);
	}
	// GPIO0-10 input
	pio_sm_set_consecutive_pindirs(pio, sm, base_pin, 11, false);
	// state machine 初期化
	pio_sm_init(pio, sm, offset, &c);
	// enable
	pio_sm_set_enabled(pio, sm, true);
}

// io_data
// base_pin = 11
void z80_io_data_program_init(PIO pio, uint sm, uint offset, uint base_pin)
{
	pio_sm_config c = z80_io_data_program_get_default_config(offset);
	sm_config_set_in_pins(&c, base_pin);
	sm_config_set_out_pins(&c, base_pin, 8);
	// inでISRに左シフトでデータ格納
	sm_config_set_in_shift(&c, false, false, 32); // ISR in
	// outでOSRに右シフトでデータ出力
	sm_config_set_out_shift(&c, true, false, 32); // OSR out
	sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_NONE);
	// 4分周
	sm_config_set_clkdiv(&c, 4.0f);
	// GPIO11-18をPIO管轄に
	for (int i = base_pin; i < base_pin + 8; ++ i)
	{
		pio_gpio_init(pio, i);
	}
	// GP11-GP18 input
	pio_sm_set_consecutive_pindirs(pio, sm, base_pin, 8, false);
	// state machine 初期化
	pio_sm_init(pio, sm, offset, &c);
	// enable
	pio_sm_set_enabled(pio, sm, true);
}
