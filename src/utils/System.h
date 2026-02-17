#pragma once

#include <cstddef>

#include "esp_system.h"
inline size_t getFreeHeap()
{
    return esp_get_free_heap_size();
}
inline void systemRestart()
{
    esp_restart();
}
