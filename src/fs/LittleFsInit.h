#pragma once

#include "../config.h"
#include <cstddef>

#ifdef USE_ESP_IDF
#include "esp_littlefs.h"
#include "esp_log.h"

#define LITTLEFS_MOUNT_POINT "/littlefs"
#define LITTLEFS_PARTITION_LABEL "littlefs"

static const char *LITTLEFS_TAG = "LittleFS";

inline bool initLittleFs()
{
    esp_vfs_littlefs_conf_t conf = {};
    conf.base_path = LITTLEFS_MOUNT_POINT;
    conf.partition_label = LITTLEFS_PARTITION_LABEL;
    conf.format_if_mount_failed = true;
    conf.dont_mount = false;

    esp_err_t ret = esp_vfs_littlefs_register(&conf);
    if (ret != ESP_OK)
    {
        ESP_LOGE(LITTLEFS_TAG, "Failed to mount LittleFS (%s)", esp_err_to_name(ret));
        return false;
    }

    size_t total = 0, used = 0;
    esp_littlefs_info(LITTLEFS_PARTITION_LABEL, &total, &used);
    ESP_LOGI(LITTLEFS_TAG, "LittleFS mounted: total=%u, used=%u", (unsigned)total, (unsigned)used);
    return true;
}

inline void deinitLittleFs()
{
    esp_vfs_littlefs_unregister(LITTLEFS_PARTITION_LABEL);
}

inline bool formatLittleFs()
{
    esp_err_t ret = esp_littlefs_format(LITTLEFS_PARTITION_LABEL);
    return ret == ESP_OK;
}

inline size_t getLittleFsTotalBytes()
{
    size_t total = 0, used = 0;
    if (esp_littlefs_info(LITTLEFS_PARTITION_LABEL, &total, &used) == ESP_OK)
    {
        return total;
    }
    return 0;
}

inline size_t getLittleFsUsedBytes()
{
    size_t total = 0, used = 0;
    if (esp_littlefs_info(LITTLEFS_PARTITION_LABEL, &total, &used) == ESP_OK)
    {
        return used;
    }
    return 0;
}

#else // Arduino

#include <LittleFS.h>

inline bool initLittleFs()
{
    return LittleFS.begin();
}

inline void deinitLittleFs()
{
    LittleFS.end();
}

inline bool formatLittleFs()
{
    return LittleFS.format();
}

inline size_t getLittleFsTotalBytes()
{
    return LittleFS.totalBytes();
}

inline size_t getLittleFsUsedBytes()
{
    return LittleFS.usedBytes();
}

#endif
