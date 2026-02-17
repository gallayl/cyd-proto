#pragma once

#include "../../config.h"

#if ENABLE_OTA

#include "../Feature.h"

#ifdef USE_ESP_IDF

#define REFRESH_TIMEOUT_AFTER_UPDATE "30"

// ESP-IDF OTA will be implemented in Phase 5 (esp_ota_ops)
extern Feature *otaUpgrade;

#else // Arduino

#include "ESPAsyncWebServer.h"
#include <Update.h>

#define REFRESH_TIMEOUT_AFTER_UPDATE "30"

// handler variables defined in OTA.cpp
extern ArRequestHandlerFunction getUpdateForm;
extern ArRequestHandlerFunction getRedirectPage;
extern ArRequestHandlerFunction onPostUpdate;
extern ArUploadHandlerFunction onUploadUpdate;

// feature object
extern Feature *otaUpgrade;

#endif // USE_ESP_IDF

#endif // ENABLE_OTA
