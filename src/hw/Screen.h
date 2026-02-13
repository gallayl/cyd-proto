#pragma once

#include <LovyanGFX.hpp>
#include "./LovyanGFX_ILI9341_Settings.h"

LGFX tft;

void initScreen()
{
    tft.init();
}