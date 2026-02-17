#pragma once

#include "../config.h"

#if ENABLE_WEBSERVER

#ifdef USE_ESP_IDF

#include <string>

void initWebSockets();
void wsBroadcast(const std::string &msg);
void wsOnSessionClose(int sockfd);

#else // Arduino

#include <ESPAsyncWebServer.h>
#include "../ActionRegistry/ActionRegistry.h"

extern AsyncWebSocket *webSocket;

void initWebSockets();

#endif

#endif
