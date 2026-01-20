#include "hw_config.h"

// SPI1 setting
static spi_t spis[] = {
    {
        .hw_inst = spi1,
        .mosi_gpio = 27,
        .miso_gpio = 28,
        .sck_gpio = 26,
        .baud_rate = 12 * 1000 * 1000,
        .set_drive_strength = false
    }
};

// SPI interface setting
static sd_spi_if_t spi_ifs[] = {
    {
        .spi = &spis[0],
        .ss_gpio = 19,
        .set_drive_strength = false
    }
};

// SD card setting
static sd_card_t sd_cards[] = {
    {
        .type = SD_IF_SPI,
        .spi_if_p = &spi_ifs[0],
        .use_card_detect = false,
        .card_detect_gpio = 0,
        .card_detected_true = 0,
        .card_detect_use_pull = false,
        .card_detect_pull_hi = false
    }
};

size_t sd_get_num() { return count_of(sd_cards); }
sd_card_t *sd_get_by_num(size_t num) { return &sd_cards[num]; }
size_t spi_get_num() { return count_of(spis); }
spi_t *spi_get_by_num(size_t num) { return &spis[num]; }
