#pragma once

#include "../config.h"

#ifndef USE_ESP_IDF
#include <ESPAsyncWebServer.h>

extern ArRequestHandlerFunction onPostUploadFiles; // defined in upload.cpp
extern ArUploadHandlerFunction uploadFiles;        // defined in upload.cpp
#endif
