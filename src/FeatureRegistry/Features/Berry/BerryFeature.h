#pragma once

#include "../../../config.h"
#include "../../Feature.h"

extern Feature *BerryFeature;

#if ENABLE_BERRY

extern "C"
{
#include "berry.h"
}

#include <vector>
#include <string>

struct BerryScriptInfo
{
    std::string name;
    std::string path;
    std::string startMenu;
    std::string iconType;  // "builtin", "file", "procedural", or empty
    std::string iconValue; // builtin name, file path, or empty
};

BerryScriptInfo parseAppMetadata(const std::string &path);

bvm *getBerryVM();
std::vector<BerryScriptInfo> scanBerryScripts(const char *dir = "/berry/apps");
void openBerryScript(const std::string &filePath);
void openBerryPanel(const std::string &filePath);

#endif // ENABLE_BERRY
