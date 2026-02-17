#pragma once

#ifdef USE_ESP_IDF
#include "cJSON.h"
#else
#include <ArduinoJson.h>
#endif
#include "../../config.h"
#include "../Feature.h"

#ifdef USE_ESP_IDF
// Returns a cJSON object. Caller must free with cJSON_Delete().
cJSON *getInfo();
#else
JsonDocument getInfo();
#endif

extern Feature *systemFeatures;
