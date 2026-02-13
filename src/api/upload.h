#pragma once
#include "../FeatureRegistry/Features/Logging.h"
#include <ESPAsyncWebServer.h>

extern ArRequestHandlerFunction onPostUploadFiles; // defined in upload.cpp

extern ArUploadHandlerFunction uploadFiles; // defined in upload.cpp