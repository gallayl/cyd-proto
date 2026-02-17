#pragma once

#ifdef USE_ESP_IDF
#include "cJSON.h"
#else
#include <ArduinoJson.h>
#endif

#include <mutex>
#include <functional>

#ifdef USE_ESP_IDF
extern cJSON *registeredFeatures;
#else
extern JsonDocument registeredFeatures;
#endif

extern std::mutex registeredFeaturesMutex;

#ifdef USE_ESP_IDF
inline void withRegisteredFeatures(std::function<void(cJSON *)> fn)
{
    std::lock_guard<std::mutex> lock(registeredFeaturesMutex);
    fn(registeredFeatures);
}
#else
inline void withRegisteredFeatures(std::function<void(const JsonDocument &)> fn)
{
    std::lock_guard<std::mutex> lock(registeredFeaturesMutex);
    fn(registeredFeatures);
}
#endif
