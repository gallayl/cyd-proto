#pragma once

#include "../config.h"

#if ENABLE_WEBSERVER

#include "../hw/WiFi.h"

#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include "./FeatureRegistry/Features/Logging.h"
#include "../mime.h"
#include "../api/upload.h"
#include "../api/list.h"

extern AsyncWebServer server;

void initWebServer();

#endif
