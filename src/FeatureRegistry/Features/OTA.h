#pragma once

#include "../../config.h"
#include "ESPAsyncWebServer.h"

#if ENABLE_OTA

#include <Update.h>
#include "../Feature.h"

#define REFRESH_TIMEOUT_AFTER_UPDATE "30"

// handler variables defined in OTA.cpp
extern ArRequestHandlerFunction getUpdateForm;
extern ArRequestHandlerFunction getRedirectPage;
extern ArRequestHandlerFunction onPostUpdate;
extern ArUploadHandlerFunction onUploadUpdate;

// websocket used during OTA

// feature object
extern Feature *otaUpgrade;

#endif
