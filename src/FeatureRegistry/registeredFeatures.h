#pragma once

#include "cJSON.h"

#include <mutex>
#include <functional>

extern cJSON *registeredFeatures;

extern std::mutex registeredFeaturesMutex;

inline void withRegisteredFeatures(std::function<void(cJSON *)> fn)
{
    std::lock_guard<std::mutex> lock(registeredFeaturesMutex);
    fn(registeredFeatures);
}
