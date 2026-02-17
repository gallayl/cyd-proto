#pragma once

#include "../../config.h"

#if ENABLE_OTA

#include "../Feature.h"

#define REFRESH_TIMEOUT_AFTER_UPDATE "30"

extern Feature *otaUpgrade;

#endif // ENABLE_OTA
