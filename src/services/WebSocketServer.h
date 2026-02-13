#pragma once

#include <ESPAsyncWebServer.h>
#include "../config.h"
#include "./WebServer.h"
#include "../CommandInterpreter/CommandInterpreter.h"
#include "../FeatureRegistry/Features/Logging.h"

extern AsyncWebSocket *webSocket;

// implementation in WebSocketServer.cpp
void initWebSockets();