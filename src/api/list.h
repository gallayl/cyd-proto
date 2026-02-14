#pragma once

#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "../mime.h"

JsonDocument getFileList();
JsonDocument getFileList(const char *path);

extern ArRequestHandlerFunction listFiles; // defined in list.cpp
