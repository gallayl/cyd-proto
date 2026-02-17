#include "registeredFeatures.h"

cJSON *registeredFeatures = cJSON_CreateObject();

std::mutex registeredFeaturesMutex;
