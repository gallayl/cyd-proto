#pragma once

#include "../config.h"

#if ENABLE_WEBSERVER

#ifndef USE_ESP_IDF
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#endif

#include "./FeatureRegistry/Features/Logging.h"

#ifndef USE_ESP_IDF
extern AsyncWebServer server;
#endif

void initWebServer();

#endif
