#pragma once

#include "../config.h"

#if ENABLE_WEBSERVER

#include <esp_http_server.h>

httpd_handle_t getHttpServer();
void stopWebServer();

#include "./FeatureRegistry/Features/Logging.h"

void initWebServer();

#endif
