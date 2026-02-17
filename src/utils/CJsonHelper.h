#pragma once

#include "cJSON.h"
#include <string>
#include <memory>

struct CJsonDeleter
{
    void operator()(cJSON *p) const
    {
        if (p)
            cJSON_Delete(p);
    }
};

using CJsonPtr = std::unique_ptr<cJSON, CJsonDeleter>;

inline std::string cJsonToString(const cJSON *json)
{
    if (!json)
        return "{}";
    char *str = cJSON_PrintUnformatted(json);
    if (!str)
        return "{}";
    std::string result(str);
    cJSON_free(str);
    return result;
}
