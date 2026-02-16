#pragma once

#include "../../../config.h"

#if ENABLE_SD_CARD

#include <cstdint>
#include "../../Feature.h"
#include "../../../ActionRegistry/FeatureAction.h"
#include "../Logging.h"

#ifdef USE_ESP_IDF
#define SD_MOUNT_POINT "/sdcard"
#endif

extern Feature *SdCardFeature;

bool isSdCardMounted();

// Platform-independent SD card info accessors
const char *getSdCardTypeName();
uint64_t getSdCardTotalBytes();
uint64_t getSdCardUsedBytes();
uint64_t getSdCardSize();

#endif // ENABLE_SD_CARD
