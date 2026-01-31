#include <cstdio>
#include <cstring>
#include "pico/stdlib.h"
#include "hardware/vreg.h"
#include "hardware/clocks.h"
#include "sdcard.hpp"
#include "uart.hpp"
#include "pio.hpp"
#include "psram.hpp"
#include "z80_io_sampling.pio.h"
#include "z80_in_data.pio.h"

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

// PIO-3034 EMM1 320KB/8MB
// MZ-1R13  KANJIROM

#define USE_PSRAM

static const uint LED_PIN = PICO_DEFAULT_LED_PIN;
static const uint IORQ = 0x1;
static const uint RD = 0x2;
static const uint WR = 0x4;
static const uint ADDRESS_SHIFT = 3;
static const uint ADDRESS_MASK = 0xFF << ADDRESS_SHIFT;
static const uint DATA_SHIFT = 11;
static const uint DATA_MASK = 0xFF << DATA_SHIFT;

// PIO-3034 EMM
#ifdef USE_PSRAM
static const uint EMM_SIZE = PSRAM_SIZE;	// 8MB
uint8_t* emmData = (uint8_t*)PSRAM_BASE;
#else
static const uint EMM_SIZE = 0x50000;		// 320KB
uint8_t __attribute__((aligned(4))) emmData[EMM_SIZE];
#endif

// MZ-1R13 KanjiROM
// ビット反転テーブル
static const uint8_t bitRevTable[256] __attribute__((section(".tcm_data"))) =
{
    0x00,0x80,0x40,0xC0,0x20,0xA0,0x60,0xE0,0x10,0x90,0x50,0xD0,0x30,0xB0,0x70,0xF0,
    0x08,0x88,0x48,0xC8,0x28,0xA8,0x68,0xE8,0x18,0x98,0x58,0xD8,0x38,0xB8,0x78,0xF8,
    0x04,0x84,0x44,0xC4,0x24,0xA4,0x64,0xE4,0x14,0x94,0x54,0xD4,0x34,0xB4,0x74,0xF4,
    0x0C,0x8C,0x4C,0xCC,0x2C,0xAC,0x6C,0xEC,0x1C,0x9C,0x5C,0xDC,0x3C,0xBC,0x7C,0xFC,
    0x02,0x82,0x42,0xC2,0x22,0xA2,0x62,0xE2,0x12,0x92,0x52,0xD2,0x32,0xB2,0x72,0xF2,
    0x0A,0x8A,0x4A,0xCA,0x2A,0xAA,0x6A,0xEA,0x1A,0x9A,0x5A,0xDA,0x3A,0xBA,0x7A,0xFA,
    0x06,0x86,0x46,0xC6,0x26,0xA6,0x66,0xE6,0x16,0x96,0x56,0xD6,0x36,0xB6,0x76,0xF6,
    0x0E,0x8E,0x4E,0xCE,0x2E,0xAE,0x6E,0xEE,0x1E,0x9E,0x5E,0xDE,0x3E,0xBE,0x7E,0xFE,
    0x01,0x81,0x41,0xC1,0x21,0xA1,0x61,0xE1,0x11,0x91,0x51,0xD1,0x31,0xB1,0x71,0xF1,
    0x09,0x89,0x49,0xC9,0x29,0xA9,0x69,0xE9,0x19,0x99,0x59,0xD9,0x39,0xB9,0x79,0xF9,
    0x05,0x85,0x45,0xC5,0x25,0xA5,0x65,0xE5,0x15,0x95,0x55,0xD5,0x35,0xB5,0x75,0xF5,
    0x0D,0x8D,0x4D,0xCD,0x2D,0xAD,0x6D,0xED,0x1D,0x9D,0x5D,0xDD,0x3D,0xBD,0x7D,0xFD,
    0x03,0x83,0x43,0xC3,0x23,0xA3,0x63,0xE3,0x13,0x93,0x53,0xD3,0x33,0xB3,0x73,0xF3,
    0x0B,0x8B,0x4B,0xCB,0x2B,0xAB,0x6B,0xEB,0x1B,0x9B,0x5B,0xDB,0x3B,0xBB,0x7B,0xFB,
    0x07,0x87,0x47,0xC7,0x27,0xA7,0x67,0xE7,0x17,0x97,0x57,0xD7,0x37,0xB7,0x77,0xF7,
    0x0F,0x8F,0x4F,0xCF,0x2F,0xAF,0x6F,0xEF,0x1F,0x9F,0x5F,0xDF,0x3F,0xBF,0x7F,0xFF
};
extern const uint8_t _binary_data_MZ1R13_KAN_ROM_start[];
extern const uint8_t _binary_data_MZ1R13_DIC_ROM_start[];

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

// MZ-1R13 data
static inline __attribute__((always_inline)) uint8_t getKanjiData(uint32_t address, uint32_t select, uint8_t ioAddress)
{
	uint32_t offset = (address << 1) | (ioAddress & 1);
	uint8_t value = select ? _binary_data_MZ1R13_KAN_ROM_start[offset & 0x1FFFF] : _binary_data_MZ1R13_DIC_ROM_start[offset & 0x3FFF];
	return value;
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
	uint sm_rx = 0;
	uint sm_tx = 1;
	uint offset_rx = pio_add_program(pio, &z80_io_sampling_program);
	uint offset_tx = pio_add_program(pio, &z80_in_data_program);
	// GPIO0～
	z80_io_sampling_program_init(pio, sm_rx, offset_rx, 0);
	// GPIO11～
	z80_in_data_program_init(pio, sm_tx, offset_tx, 11);
	pio_sm_set_enabled(pio, sm_rx, true);
	pio_sm_set_enabled(pio, sm_tx, true);

#ifdef USE_PSRAM
	// init PSRAM
	uint psramsize = psram_init(PIMORONI_PICO_PLUS2_PSRAM_CS_PIN);
#endif

	// init device
	uint32_t allGpio;
	uint8_t ioAddress = 0;
	uint8_t data = 0xFF;
	uint8_t value = 0;
	uint32_t emmAddress = 0;
	uint32_t kanjiAddress = 0;
	uint32_t kanjiSelect = 0;
	int toggle = 1;
	bool flag;
	memset(emmData, 0, EMM_SIZE);

	char msg[1024];
	int debugIndex = 0;

	do
	{
		// Wait while /IORQ is high
		allGpio = pio_sm_get_blocking(pio, sm_rx);

//		sprintf(msg, "allGpio: %x\r\n", allGpio);
//		uart_puts(UART_ID, msg);

		// I/O Read, /RD is Low
		if(!(allGpio & RD))
		{
			ioAddress = (allGpio >> ADDRESS_SHIFT) & 0xFF;
			switch(ioAddress)
			{
				// Debug
			case 0:
				data = emmData[emmAddress];
				// output data
				pio_sm_put_blocking(pio, sm_tx, data);
				// add address
				++ emmAddress;
				break;
			case 1:
				pio_sm_put_blocking(pio, sm_tx, data);
				debugDump(emmData, 256);
				sprintf(msg, "emmAddress: %u \r\n", emmAddress);
				uart_puts(UART_ID, msg);
				break;
				// PIO-3034 EMM1
			case 0xA7:
				data = emmData[emmAddress];
				// output data
				pio_sm_put_blocking(pio, sm_tx, data);
				// add address
				++ emmAddress;
				if(emmAddress >= EMM_SIZE)
				{
					emmAddress -= EMM_SIZE;
				}
				break;
				// MZ-1R13 KANJIROM
			case 0xB8:
				data = getKanjiData(kanjiAddress, kanjiSelect, ioAddress);
				// output data
				pio_sm_put_blocking(pio, sm_tx, data);
				break;
			case 0xB9:
				data = getKanjiData(kanjiAddress, kanjiSelect, ioAddress);
				// output data
				pio_sm_put_blocking(pio, sm_tx, data);
				++ kanjiAddress;
				break;
			case 0xBA:
				data = bitRevTable[getKanjiData(kanjiAddress, kanjiSelect, ioAddress)];
				// output data
				pio_sm_put_blocking(pio, sm_tx, data);
				break;
			case 0xBB:
				data = bitRevTable[getKanjiData(kanjiAddress, kanjiSelect, ioAddress)];
				// output data
				pio_sm_put_blocking(pio, sm_tx, data);
				++ kanjiAddress;
				break;
			}
		}
		// I/O Write, /WR is low
		else if(!(allGpio & WR))
		{
			ioAddress = (allGpio >> ADDRESS_SHIFT) & 0xFF;
			data = (allGpio >> DATA_SHIFT) & 0xFF;
			switch(ioAddress)
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
				debugDump((void*)_binary_data_MZ1R13_KAN_ROM_start, 256);
				sprintf(msg, "kanjiAddress: %u, kanjiSelect: %u \r\n", kanjiAddress, kanjiSelect);
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
				// PIO-3034 EMM1
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
				// MZ-1R13 KANJIROM
			case 0xB8:
				kanjiAddress = (kanjiAddress & 0xFF00) | data;
				break;
			case 0xB9:
				kanjiAddress = (kanjiAddress & 0x00FF) | (data << 8);
				break;
			case 0xBA:
				kanjiSelect = ((data & 1) != 0);
				break;
			case 0xBB:
				++ kanjiAddress;
				break;
			}
		}
	}
	while(true);

	return 0;
}
