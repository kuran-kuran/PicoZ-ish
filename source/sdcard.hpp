#ifndef SDCARD_HPP
#define SDCARD_HPP

#include "sd_card.h"

bool sdInit(void);
bool sdSave(const char* filePath, const void* buffer, FSIZE_t size);
bool sdLoad(const char* filePath, void* buffer, FSIZE_t bufferSize);
bool sdAppend(const char* filePath, const char* text);

#endif
