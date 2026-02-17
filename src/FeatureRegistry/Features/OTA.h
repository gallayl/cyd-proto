#pragma once

#include "../../config.h"

#if ENABLE_OTA

#include "../Feature.h"

#define REFRESH_TIMEOUT_AFTER_UPDATE "30"

#ifdef USE_ESP_IDF

extern Feature *otaUpgrade;

#else // Arduino

#include "ESPAsyncWebServer.h"
#include <Update.h>

extern ArRequestHandlerFunction getUpdateForm;
extern ArRequestHandlerFunction getRedirectPage;
extern ArRequestHandlerFunction onPostUpdate;
extern ArUploadHandlerFunction onUploadUpdate;

extern Feature *otaUpgrade;

#endif // USE_ESP_IDF

#endif // ENABLE_OTA
