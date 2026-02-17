#pragma once

#include "../../../config.h"

#if ENABLE_SD_CARD

#include <cstdint>
#include "../../Feature.h"
#include "../../../ActionRegistry/FeatureAction.h"
#include "../Logging.h"

#define SD_MOUNT_POINT "/sdcard"

extern Feature *SdCardFeature;

bool isSdCardMounted();

// Platform-independent SD card info accessors
const char *getSdCardTypeName();
uint64_t getSdCardTotalBytes();
uint64_t getSdCardUsedBytes();
uint64_t getSdCardSize();

#endif // ENABLE_SD_CARD
