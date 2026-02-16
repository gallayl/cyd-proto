#pragma once

#include <cstddef>

#ifdef USE_ESP_IDF
#include "esp_system.h"
inline size_t getFreeHeap()
{
    return esp_get_free_heap_size();
}
inline void systemRestart()
{
    esp_restart();
}
#else
#include <Esp.h>
inline size_t getFreeHeap()
{
    return ESP.getFreeHeap();
}
inline void systemRestart()
{
    ESP.restart();
}
#endif
