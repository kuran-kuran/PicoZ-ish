#include <cstdio>
#include <cstring>
#include "pico/stdlib.h"
#include "hardware/vreg.h"
#include "hardware/clocks.h"
#include "sdcard.hpp"
#include "uart.hpp"
#include "pio.hpp"
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
// GPIO19 SPI1 SD-Card CS
// GPIO20 UART TX
// GPIO21 UART RX
// GPIO22 Reserved
// GPIO26 SPI1 SD-Card SCK
// GPIO27 SPI1 SD-Card MOSI
// GPIO28 SPI1 SD-Card MISO

static const uint LED_PIN = PICO_DEFAULT_LED_PIN;
static const uint IORQ = 0x1;
static const uint RD = 0x2;
static const uint WR = 0x4;
static const uint ADDRESS_SHIFT = 3;
static const uint ADDRESS_MASK = 0xFF << ADDRESS_SHIFT;
//static const uint DATA_SHIFT = 11;
//static const uint DATA_MASK = 0xFF << DATA_SHIFT;
static const uint EMM_SIZE = 0x50000; // 320KB

uint8_t __attribute__((aligned(4))) emmData[EMM_SIZE];

void debugDump(void* buffer, int size)
{
	unsigned char* buffer8 = (unsigned char*)buffer;
	char text[64];
	uart_puts(UART_ID, "\r\n");
	for(int i = 0; i < size; ++ i)
	{
		if((i % 16) == 15)
		{
			sprintf(text, "%02X\r\n", buffer8[i]);
		}
		else
		{
			sprintf(text, "%02X ", buffer8[i]);
		}
		uart_puts(UART_ID, text);
	}
	uart_puts(UART_ID, "\r\n");
}

__attribute__((noinline)) int __time_critical_func(main)(void)
{
	stdio_init_all();

	// init GPIO
    gpio_init_mask(0xFFFFFFFF);

	// init SD card
	sdInit();

	// overclock 300MHz
	vreg_set_voltage(VREG_VOLTAGE_1_20);
	set_sys_clock_khz(300000 ,true);

	// init UART
	uartInit();
	uart_puts(UART_ID, "Start.\r\n");

	// GPIO0 - GPIO18 pins are INPUT
	gpio_set_dir_masked(0x7FFFF, 0x00000);

	// Debug
	gpio_init(LED_PIN);
	gpio_set_dir(LED_PIN, GPIO_OUT);
	gpio_put(LED_PIN, true);

	// io_sampling
	PIO pio = pio0;
	uint sm_address = 0;
	uint sm_data = 1;
	uint offset_address = pio_add_program(pio, &z80_io_address_program);
	uint offset_data = pio_add_program(pio, &z80_io_data_program);
	// GPIO0～
	z80_io_address_program_init(pio, sm_address, offset_address, 0);
	// GPIO11～
	z80_io_data_program_init(pio, sm_data, offset_data, 11);

	// init device
	uint32_t bus;
	uint32_t address;
	uint32_t data;
//	uint8_t ioAddress = 0;
//	uint8_t data = 0xFF;
	uint32_t emmAddress = 0;
	int toggle = 1;
	bool flag;
	memset(emmData, 0, EMM_SIZE);

	char msg[1024];

	do
	{
		// Wait while /IORQ is high
		bus = pio_sm_get_blocking(pio, sm_address);

//		sprintf(msg, "bus: %x\r\n", bus);
//		uart_puts(UART_ID, msg);

		// I/O Read, /RD is Low
		if(!(bus & RD))
		{
			address = (bus >> ADDRESS_SHIFT) & 0xFF;

//			sprintf(msg, "in address: %x\r\n", address);
//			uart_puts(UART_ID, msg);

			switch(address)
			{
				// Debug
			case 0:
				data = 5 << 1;//(emmData[emmAddress] << 1); //@@

			sprintf(msg, "in address: %x, data %x\r\n", address, data);
			uart_puts(UART_ID, msg);

				// output data
				pio_sm_put_blocking(pio, sm_data, data);
				// add address
				++ emmAddress;
				break;
				// EMM1
			case 0xA7:
				data = (emmData[emmAddress] << 1);
				// output data
				pio_sm_put_blocking(pio, sm_data, data);
				// add address
				++ emmAddress;
				if(emmAddress >= EMM_SIZE)
				{
					emmAddress -= EMM_SIZE;
				}
				break;
			}
		}
		// I/O Write, /WR is low
		else if(!(bus & WR))
		{
			address = (bus >> ADDRESS_SHIFT) & 0xFF;
			// request data
			pio_sm_put_blocking(pio, sm_data, 0);
			// receive data
			data = pio_sm_get_blocking(pio, sm_data);// & 0xFF;

//			sprintf(msg, "out address: %x, data: %x\r\n", address, data);
//			uart_puts(UART_ID, msg);

//			sprintf(msg, "data: %x\r\n", data);
//			uart_puts(UART_ID, msg);

//			data = (allGpio >> DATA_SHIFT) & 0xFF;
			switch(address)
			{
				// Debug
			case 0:
				emmData[data] = data;
				emmAddress = 0;
				break;
			case 1:
				for(int i = 0; i < 256; ++ i)
				{
					emmData[i] = static_cast<uint8_t>(i);
				}
				emmAddress = 0;
				break;
			case 2:
				flag = true;
				for(int i = 0; i < 256; ++ i)
				{
					if(emmData[i] != static_cast<uint8_t>(i))
					{
						flag = false;
						break;
					}
				}
				if(flag == true)
				{
					toggle = 1 - toggle;
					gpio_put(LED_PIN, toggle);
				}
				debugDump(emmData, 256);
				sprintf(msg, "emmAddress: %u, data: %u \r\n", emmAddress, data);
				uart_puts(UART_ID, msg);
				break;
			case 3:
				// Save to sd-card emm memory 320KB
				toggle = 1 - toggle;
				gpio_put(LED_PIN, toggle);
				sdSave("0:EMM.bin", emmData, EMM_SIZE);
				toggle = 1 - toggle;
				gpio_put(LED_PIN, toggle);
				break;
				// EMM1
			case 0xA4:
				emmAddress = (emmAddress & 0xFFFF00) | data;
				break;
			case 0xA5:
				emmAddress = (emmAddress & 0xFF00FF) | (data << 8);
				break;
				
			case 0xA6:
				emmAddress = (emmAddress & 0x00FFFF) | (data << 16);
				break;
			case 0xA7:
				emmData[emmAddress] = data;
				++ emmAddress;
				if(emmAddress >= EMM_SIZE)
				{
					emmAddress -= EMM_SIZE;
				}
				break;
			}
		}
	}
	while(true);

	return 0;
}
