#pragma once

#include "../config.h"

#if ENABLE_SCREEN

#include <LovyanGFX.hpp>
#include "./LovyanGFX_ILI9341_Settings.h"

extern LGFX tft;

inline void initScreen()
{
    tft.init();
    tft.setBrightness(128);
}

#endif