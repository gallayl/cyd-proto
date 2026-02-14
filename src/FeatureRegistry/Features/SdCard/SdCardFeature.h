#pragma once

#include "../../../config.h"

#if ENABLE_SD_CARD

#include "../../Feature.h"
#include "../../../ActionRegistry/FeatureAction.h"
#include "../Logging.h"

extern Feature *SdCardFeature;

bool isSdCardMounted();

#endif // ENABLE_SD_CARD
