#ifndef SDCARD_H
#define SDCARD_H

#include "sd_card.h"

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif

bool sdInit(void);
bool sdSave(const char* filePath, const void* buffer, FSIZE_t size);
bool sdLoad(const char* filePath, void* buffer, FSIZE_t bufferSize);
bool sdAppend(const char* filePath, const char* text);

#ifdef __cplusplus
}
#endif

#endif
