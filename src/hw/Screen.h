#pragma once

#include <LovyanGFX.hpp>
#include "./LovyanGFX_ILI9341_Settings.h"

extern LGFX tft;

inline void initScreen()
{
    tft.init();
}