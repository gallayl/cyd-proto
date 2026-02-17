#pragma once

#include "../config.h"

#if ENABLE_WEBSERVER

#ifdef USE_ESP_IDF
#include <esp_http_server.h>

httpd_handle_t getHttpServer();
void stopWebServer();
#else
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>

extern AsyncWebServer server;
#endif

#include "./FeatureRegistry/Features/Logging.h"

void initWebServer();

#endif
