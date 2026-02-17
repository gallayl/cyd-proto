#include "registeredFeatures.h"

#ifdef USE_ESP_IDF
cJSON *registeredFeatures = cJSON_CreateObject();
#else
#include <ArduinoJson.h>
JsonDocument registeredFeatures;
#endif

std::mutex registeredFeaturesMutex;
