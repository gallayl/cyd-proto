#pragma once

#ifdef USE_ESP_IDF

#include "cJSON.h"

inline void merge(cJSON *dst, const cJSON *src)
{
    if (!dst || !src)
        return;

    if (cJSON_IsObject(src))
    {
        const cJSON *item = nullptr;
        cJSON_ArrayForEach(item, src)
        {
            cJSON *dstItem = cJSON_GetObjectItem(dst, item->string);
            if (dstItem && cJSON_IsObject(dstItem) && cJSON_IsObject(item))
            {
                merge(dstItem, item);
            }
            else
            {
                cJSON *dup = cJSON_Duplicate(item, true);
                if (dstItem)
                {
                    cJSON_ReplaceItemInObject(dst, item->string, dup);
                }
                else
                {
                    cJSON_AddItemToObject(dst, item->string, dup);
                }
            }
        }
    }
}

#else

#include <ArduinoJson.h>

inline void merge(JsonObject dst, JsonVariantConst src)
{
    if (src.is<JsonObjectConst>())
    {
        for (JsonPairConst kvp : src.as<JsonObjectConst>())
        {
            if (dst[kvp.key()].is<JsonObject>() && kvp.value().is<JsonObject>())
            {
                merge(dst[kvp.key()], kvp.value());
            }
            else
            {
                dst[kvp.key()] = kvp.value();
            }
        }
    }
    else
    {
        dst.set(src);
    }
}

#endif
