#pragma once

#include "cJSON.h"
#include "../../config.h"
#include "../Feature.h"

// Returns a cJSON object. Caller must free with cJSON_Delete().
cJSON *getInfo();

extern Feature *systemFeatures;
