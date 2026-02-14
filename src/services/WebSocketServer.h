#pragma once

#include "../config.h"

#if ENABLE_WEBSERVER

#include <ESPAsyncWebServer.h>
#include "./WebServer.h"
#include "../CommandInterpreter/CommandInterpreter.h"
#include "../FeatureRegistry/Features/Logging.h"

extern AsyncWebSocket *webSocket;

// implementation in WebSocketServer.cpp
void initWebSockets();

#endif
