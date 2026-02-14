#pragma once

#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <FS.h>
#include "../mime.h"

JsonDocument getFileList();
JsonDocument getFileList(const char *path);
JsonDocument getFileList(fs::FS &filesystem, const char *path);

extern ArRequestHandlerFunction listFiles; // defined in list.cpp
