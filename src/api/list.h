#pragma once

#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "../mime.h"

JsonDocument getFileList();

extern ArRequestHandlerFunction listFiles; // defined in list.cpp
