extern "C"
{
#include "ff.h"
}
#include "hw_config.h"
#include "sdcard.hpp"
#include <cstring>

// GPIO19 SPI1 SD-Card CS
// GPIO26 SPI1 SD-Card SCK
// GPIO27 SPI1 SD-Card MOSI
// GPIO28 SPI1 SD-Card MISO

bool sdInit(void)
{
	sd_card_t *sd = sd_get_by_num(0);
	sd_spi_if_t* spi_if = sd->spi_if_p;
	spi_t* spi_cfg = spi_if->spi;
	// SPI setting
	gpio_init(spi_cfg->miso_gpio);
	gpio_init(spi_cfg->mosi_gpio);
	gpio_init(spi_cfg->sck_gpio);
	spi_init(spi_cfg->hw_inst, 50 * 1000 * 1000);
	gpio_set_function(spi_cfg->sck_gpio,  GPIO_FUNC_SPI);
	gpio_set_function(spi_cfg->mosi_gpio, GPIO_FUNC_SPI);
	gpio_set_function(spi_cfg->miso_gpio, GPIO_FUNC_SPI);
	// CS pin
	gpio_init(spi_if->ss_gpio);
	gpio_set_dir(spi_if->ss_gpio, GPIO_OUT);
	gpio_put(spi_if->ss_gpio, 1);
	// SD setting
	FRESULT fr = f_mount(&sd->state.fatfs, "0:", 1);
	if(fr != FR_OK)
	{
		return false;
	}
	return true;
}

bool sdSave(const char* filePath, const void* buffer, FSIZE_t size)
{
	FIL fp;
	UINT bw;
	FRESULT fr = f_open(&fp, filePath, FA_WRITE | FA_CREATE_ALWAYS);
	if(fr != FR_OK)
	{
		return false;
	}
	fr = f_write(&fp, buffer, size, &bw);
	f_close(&fp);
	if((fr != FR_OK) || (bw != size))
	{
		return false;
	}
	return true;
}

bool sdLoad(const char* filePath, void* buffer, FSIZE_t bufferSize)
{
	FIL fp;
	UINT br;
	FRESULT fr = f_open(&fp, filePath, FA_READ);
	if(fr != FR_OK)
	{
		return false;
	}
	FSIZE_t fileSize = f_size(&fp);
	FSIZE_t readSize = fileSize;
	if(fileSize > bufferSize)
	{
		readSize = bufferSize;
	}
	fr = f_read(&fp, buffer, readSize, &br);
	f_close(&fp);
	if(fr != FR_OK)
	{
		return false;
	}
	return true;
}

bool sdAppend(const char* filePath, const char* text)
{
	FIL fp;
	UINT bw;
	FRESULT fr = f_open(&fp, filePath, FA_WRITE | FA_OPEN_APPEND);
	if(fr != FR_OK)
	{
		return false;
	}
	fr = f_write(&fp, text, strlen(text), &bw);
	f_close(&fp);
	if(fr != FR_OK || bw != strlen(text))
	{
		return false;
	}
	return true;
}
