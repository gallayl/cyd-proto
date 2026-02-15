#pragma once

#include "../../../config.h"

#if ENABLE_BERRY

extern "C"
{
#include "berry.h"
}

#include "../UI/elements/container.h"
#include <vector>

class BerryApp;

// --- Handle table: maps integer handles to UI::Element* ---

BerryApp *berryCurrentApp();
void berrySetCurrentApp(BerryApp *app);

// Register the 'ui' module as a Berry global
void registerBerryUIModule(bvm *vm);

// Get Berry VM (defined in BerryFeature.cpp)
bvm *getBerryVM();

#endif // ENABLE_BERRY
