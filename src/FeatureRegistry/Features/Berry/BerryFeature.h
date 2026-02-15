#pragma once

#include "../../../config.h"
#include "../../Feature.h"
#include "../../../ActionRegistry/FeatureAction.h"

extern Feature *BerryFeature;

#if ENABLE_BERRY

extern "C"
{
#include "berry.h"
}

#include <vector>
#include <Arduino.h>

struct BerryScriptInfo
{
    String name;
    String path;
};

bvm *getBerryVM();
std::vector<BerryScriptInfo> scanBerryScripts(const char *dir = "/berry/apps");
void openBerryScript(const String &filePath);
void openBerryPanel(const String &filePath);

#endif // ENABLE_BERRY
