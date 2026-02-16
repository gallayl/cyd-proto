#pragma once

#include <ArduinoJson.h>
#include "../../config.h"
#include "../Feature.h"

JsonDocument getInfo();

extern Feature *systemFeatures;
