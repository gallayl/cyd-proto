#pragma once

#include "../../../config.h"
#include "../../Feature.h"
#include "../../../ActionRegistry/FeatureAction.h"

extern Feature *BerryFeature;

#if ENABLE_BERRY

extern "C" {
#include "berry.h"
}

#include <vector>
#include <Arduino.h>

bvm *getBerryVM();
const std::vector<String> &getBerryAppNames();

#endif // ENABLE_BERRY
