#include <cstdio>
#include <cstring>
#include "pico/stdlib.h"
#include "hardware/vreg.h"
#include "hardware/clocks.h"

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

static const uint LED_PIN = PICO_DEFAULT_LED_PIN;
static const uint WAIT_RD = 0x2;
static const uint WAIT_WR = 0x4;
static const uint CONTROL_MASK = 0x7;
static const uint CONTROL_READ = 0x4;
static const uint CONTROL_WRITE = 0x2;
static const uint ADDRESS_SHIFT = 3;
static const uint ADDRESS_MASK = 0xFF << ADDRESS_SHIFT;
static const uint DATA_SHIFT = 11;
static const uint DATA_MASK = 0xFF << DATA_SHIFT;
static const uint EMM_SIZE = 0x50000; // 320KB

uint8_t __attribute__  ((aligned(sizeof(uint8_t *) * 4096))) emmData[EMM_SIZE];
volatile uint32_t emmAddress = 0;
int toggle = 1;

int __not_in_flash_func(ioFunc)(void)
{
	volatile uint32_t allGpio;
	uint8_t ioAddress = 0;
	uint8_t data = 0xFF;
	uint32_t response = 0;
	uint control = 0;
	bool flag = false;
	memset(emmData, 0, EMM_SIZE);
	while(true)
	{
		allGpio = sio_hw->gpio_in;
		control = allGpio & CONTROL_MASK;
		// I/O Read
		if(control == CONTROL_READ)
		{
			ioAddress = (allGpio >> ADDRESS_SHIFT) & 0xFF;
			switch(ioAddress)
			{
			case 0:
				data = emmData[emmAddress];
				++ emmAddress;
				response = 1;
				break;
				// EMM1
			case 0xA7:
				data = emmData[emmAddress];
				++ emmAddress;
				if(emmAddress >= EMM_SIZE)
				{
					emmAddress = 0;
				}
				response = 1;
				break;
			default:
				response = 0;
			}
			control = 0;
			if(response)
			{
				// Set GPIO11-18 to Output
				gpio_set_dir_masked(DATA_MASK, DATA_MASK);
				gpio_put_masked(DATA_MASK, ((uint32_t)data << DATA_SHIFT));

				// Wait while /RD is low
				while(control == 0)
				{
					control = sio_hw->gpio_in & WAIT_RD;
				};

				// Set GPIO11-18 to Input
				gpio_set_dir_masked(DATA_MASK, 0x00);

/*
if(emmAddress == 0x20000 && data == 0) {
	toggle = 1 - toggle;
	gpio_put(LED_PIN, toggle);
}
*/

			}
			else
			{
				// Wait while /RD is low
				while(control == 0)
				{
					control = sio_hw->gpio_in & WAIT_RD;
				};
			}
			continue;
		}
		// I/O Write
		else if(control == CONTROL_WRITE)
		{
			ioAddress = (allGpio >> ADDRESS_SHIFT) & 0xFF;
			data = (allGpio >> DATA_SHIFT) & 0xFF;
			switch(ioAddress)
			{
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
					emmAddress = 0;
				}
				break;
			}
			// Wait while /WR is low
			control = 0;
			while(control == 0)
			{
				control = sio_hw->gpio_in & WAIT_WR;
			};
			continue;
		}
	}
	return 0;
}

int main()
{
//	stdio_init_all();

	// overclock 300MHz
	vreg_set_voltage(VREG_VOLTAGE_1_20);
	set_sys_clock_khz(300000 ,true);

	// All pins are INPUT
    gpio_init_mask(0xFFFFFFFF);
    gpio_set_dir_all_bits(0x00000000);

	// Debug
	gpio_init(LED_PIN);
	gpio_set_dir(LED_PIN, GPIO_OUT);
	gpio_put(LED_PIN, toggle);

	return ioFunc();
}
