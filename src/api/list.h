#pragma once

#include "../config.h"
#include <string>

#ifdef USE_ESP_IDF
#include "cJSON.h"

// Returns a cJSON array of file objects. Caller must free with cJSON_Delete().
cJSON *getFileList();
cJSON *getFileList(const char *path);
cJSON *getFileList(const std::string &realPath);
#else
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <FS.h>

JsonDocument getFileList();
JsonDocument getFileList(const char *path);
JsonDocument getFileList(fs::FS &filesystem, const char *path);

extern ArRequestHandlerFunction listFiles;
#endif
