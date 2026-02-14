#pragma once

#include "../config.h"

#if ENABLE_WEBSERVER

#include <ESPAsyncWebServer.h>
#include "./WebServer.h"
#include "../ActionRegistry/ActionRegistry.h"
#include "../FeatureRegistry/Features/Logging.h"

extern AsyncWebSocket *webSocket;

void initWebSockets();

#endif
