#pragma once

#include "../config.h"

#if ENABLE_WEBSERVER

#include <ESPAsyncWebServer.h>
#include "../ActionRegistry/ActionRegistry.h"

extern AsyncWebSocket *webSocket;

void initWebSockets();

#endif
