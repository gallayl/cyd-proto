#pragma once

#include <ArduinoJson.h>
#include <mutex>
#include <functional>

extern JsonDocument registeredFeatures;
extern std::mutex registeredFeaturesMutex;

inline void withRegisteredFeatures(std::function<void(const JsonDocument &)> fn)
{
    std::lock_guard<std::mutex> lock(registeredFeaturesMutex);
    fn(registeredFeatures);
}
