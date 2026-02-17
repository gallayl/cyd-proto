#pragma once

#include "../config.h"
#include <string>

#ifndef USE_ESP_IDF
#include <ESPAsyncWebServer.h>
#endif

#include <ArduinoJson.h>

#ifdef USE_ESP_IDF
JsonDocument getFileList();
JsonDocument getFileList(const char *path);
JsonDocument getFileList(const std::string &realPath);
#else
#include <FS.h>
JsonDocument getFileList();
JsonDocument getFileList(const char *path);
JsonDocument getFileList(fs::FS &filesystem, const char *path);

extern ArRequestHandlerFunction listFiles; // defined in list.cpp
#endif
