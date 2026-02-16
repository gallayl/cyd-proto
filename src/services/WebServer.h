#pragma once

#include "../config.h"

#if ENABLE_WEBSERVER

#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include "./FeatureRegistry/Features/Logging.h"

extern AsyncWebServer server;

void initWebServer();

#endif
