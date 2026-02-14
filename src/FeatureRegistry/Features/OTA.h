#pragma once

#include "../../config.h"

#if ENABLE_OTA

#include "./Logging.h"
#include <Update.h>
#include "../Feature.h"
#include "../../services/WebServer.h"
#include "../../services/WebSocketServer.h" // for webSocket

#define REFRESH_TIMEOUT_AFTER_UPDATE "30"

// handler variables defined in OTA.cpp
extern ArRequestHandlerFunction getUpdateForm;
extern ArRequestHandlerFunction getRedirectPage;
extern ArRequestHandlerFunction onPostUpdate;
extern ArUploadHandlerFunction onUploadUpdate;

// websocket used during OTA
extern AsyncWebSocket *webSocket;
// feature object
extern Feature *OtaUpgrade;

#endif
