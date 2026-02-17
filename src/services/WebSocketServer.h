#pragma once

#include "../config.h"

#if ENABLE_WEBSERVER

#ifndef USE_ESP_IDF
#include <ESPAsyncWebServer.h>
#include "../ActionRegistry/ActionRegistry.h"

extern AsyncWebSocket *webSocket;
#endif

void initWebSockets();

#endif
