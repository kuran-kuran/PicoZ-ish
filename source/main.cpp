#include <cstdio>
#include <cstring>
#include "pico/stdlib.h"

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

const uint LED_PIN = PICO_DEFAULT_LED_PIN;
const uint CONTROL_RD = 0x2;
const uint CONTROL_MASK = 0x7;
const uint CONTROL_READ = 0x4;
const uint CONTROL_WRITE = 0x2;
const uint ADDRESS_SHIFT = 3;
const uint ADDRESS_MASK = 0xFF << ADDRESS_SHIFT;
const uint DATA_SHIFT = 11;
const uint DATA_MASK = 0xFF << DATA_SHIFT;

uint8_t __attribute__  ((aligned(sizeof(unsigned char *) * 4096))) emmData[0x50000];
volatile uint32_t emmAddress = 0;

int __not_in_flash_func(ioFunc)(void)
{
	volatile uint32_t allGpio;
	uint8_t ioAddress = 0;
	uint8_t data = 0xFF;
	uint32_t response = 0;
	memset(emmData, 0, 0x50000);
	while(true)
	{
//		allGpio = gpio_get_all();
		allGpio = sio_hw->gpio_in;
		uint control = allGpio & CONTROL_MASK;
		ioAddress = (allGpio >> ADDRESS_SHIFT) & 0xFF;
		if(control == CONTROL_READ)
		{
			// I/O Read
			switch(ioAddress)
			{
			case 0xA7:
				data = emmData[emmAddress];
				response = 1;
				break;
			default:
				response = 0;
			}
			if(response)
			{
                // Set GPIO11-18 to Output
				gpio_set_dir_masked(DATA_MASK, DATA_MASK);
				gpio_put_masked(DATA_MASK, ((uint32_t)data << DATA_SHIFT));
				// Wait while /RD is low
				control = 0;
				while(control == 0)
				{
					allGpio = sio_hw->gpio_in;
					control = allGpio & CONTROL_RD;
				}
                // Set GPIO11-18 to Input
                gpio_set_dir_masked(DATA_MASK, 0x00);
			}
			else
			{
				// Wait while /RD is low
				control = 0;
				while(control == 0)
				{
					allGpio = sio_hw->gpio_in;
					control = allGpio & CONTROL_RD;
				}
			}
		}
		else if(control == CONTROL_WRITE)
		{
			// I/O Write
			data = (allGpio >> DATA_SHIFT) & 0xFF;
			switch(ioAddress)
			{
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
				if(emmAddress >= 0x50000)
				{
					emmAddress = 0;
				}
				break;
			}
		}
	}
}

int main()
{
	stdio_init_all();

	// All pins are INPUT
    gpio_init_mask(0xFFFFFFFF);
    gpio_set_dir_all_bits(0x00000000);

	// Debug
	gpio_init(LED_PIN);
	gpio_set_dir(LED_PIN, GPIO_OUT);
	int toggle = 1;
	gpio_put(LED_PIN, toggle);

	return ioFunc();
}
