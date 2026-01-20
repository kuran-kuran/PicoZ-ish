#include <cstdio>
#include <cstring>
#include "pico/stdlib.h"
#include "hardware/vreg.h"
#include "hardware/clocks.h"
#include "sdcard.h"
#include "uart.h"

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
static const uint DATA_SHIFT = 11;
static const uint DATA_MASK = 0xFF << DATA_SHIFT;
static const uint EMM_SIZE = 0x50000; // 320KB

uint8_t __attribute__((aligned(4))) emmData[EMM_SIZE];

// Output data
static inline __attribute__((always_inline)) void outputData(uint8_t data)
{
	// Set GPIO11-18 to Output
	gpio_set_dir_masked(DATA_MASK, DATA_MASK);
	gpio_put_masked(DATA_MASK, ((uint32_t)data << DATA_SHIFT));

	// Wait while /RD is low
	do {} while((sio_hw->gpio_in & RD) == 0);

	// Set GPIO11-18 to Input
	gpio_set_dir_masked(DATA_MASK, 0x00);
}

int __not_in_flash_func(ioFunc)(void)
{
	uint32_t allGpio;
	uint8_t ioAddress = 0;
	uint8_t data = 0xFF;
	uint32_t emmAddress = 0;
	int toggle = 1;
	bool flag;
	memset(emmData, 0, EMM_SIZE);
	do
	{
		// Wait while /IORQ is high
		do
		{
			allGpio = sio_hw->gpio_in;
		}
		while(allGpio & IORQ);
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
				outputData(data);
				// add address
				++ emmAddress;
				break;
				// EMM1
			case 0xA7:
				data = emmData[emmAddress];
				// output data
				outputData(data);
				// add address
				++ emmAddress;
				if(emmAddress >= EMM_SIZE)
				{
					emmAddress -= EMM_SIZE;
				}
				break;
			default:
				// Wait while /RD is low
				do {} while((sio_hw->gpio_in & RD) == 0);
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


/*
if(emmAddress == 0x1FFFF && data == 0) {
	toggle = 1 - toggle;
	gpio_put(LED_PIN, toggle);
}
*/

				emmData[emmAddress] = data;
				++ emmAddress;
				if(emmAddress >= EMM_SIZE)
				{
					emmAddress -= EMM_SIZE;
				}
				break;
			}
			// Wait while /WR is low
			do {} while((sio_hw->gpio_in & WR) == 0);
		}
	}
	while(true);
	return 0;
}

int main()
{
	stdio_init_all();

	// init GPIO
    gpio_init_mask(0xFFFFFFFF);

	char msg[64];
	bool r = false;

	// init SD card
	r = sdInit();
	sleep_ms(10);

	// overclock 300MHz
	vreg_set_voltage(VREG_VOLTAGE_1_20);
	set_sys_clock_khz(300000 ,true);
//	uartReInit();

	// init UART
	uartInit();
	uart_puts(UART_ID, "Start.\r\n");

	// GPIO0 - GPIO18 pins are INPUT
	gpio_set_dir_masked(0x7FFFF, 0x00000);

	// Debug
	gpio_init(LED_PIN);
	gpio_set_dir(LED_PIN, GPIO_OUT);
	gpio_put(LED_PIN, true);

	return ioFunc();
}
