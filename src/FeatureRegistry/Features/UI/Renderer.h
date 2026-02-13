#pragma once

#include <LovyanGFX.hpp>
#include "../../../hw/Screen.h"

extern LGFX tft;

namespace UI
{
    inline LGFX_Sprite &canvas()
    {
        static LGFX_Sprite sprite(&tft);
        return sprite;
    }

    inline bool initRenderer()
    {
        auto &c = canvas();
        c.setColorDepth(8);
        return c.createSprite(tft.width(), tft.height()) != nullptr;
    }

    inline void flush()
    {
        canvas().pushSprite(0, 0);
    }

} // namespace UI
