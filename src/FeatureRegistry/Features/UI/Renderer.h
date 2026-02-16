#pragma once

#include <LovyanGFX.hpp>
#include "../../../hw/Screen.h"
#include <Esp.h>
#include "../Logging.h"

extern LGFX tft;

namespace UI
{

constexpr int STRIP_H = 40;
constexpr int MAX_DIM = 320;

inline LGFX_Sprite &canvas()
{
    static LGFX_Sprite sprite(&tft);
    return sprite;
}

inline uint8_t *&stripBuffer()
{
    static uint8_t *buf = nullptr;
    return buf;
}

// Current strip Y offset. Elements subtract this from their Y when drawing.
// Default 0 means no offset (backwards compatible).
inline int &stripOffsetY()
{
    static int oy = 0;
    return oy;
}

inline bool initRenderer()
{
    size_t bufSize = MAX_DIM * STRIP_H * 2; // 16-bit
    stripBuffer() = (uint8_t *)malloc(bufSize);
    if (!stripBuffer())
    {
        if (loggerInstance)
            loggerInstance->Error("Failed to allocate strip buffer (" + String(bufSize) + " bytes)");
        return false;
    }

    auto &c = canvas();
    c.setColorDepth(16);
    // Set up sprite with max screen width and strip height
    c.setBuffer(stripBuffer(), tft.width(), STRIP_H, 16);

    if (loggerInstance)
        loggerInstance->Info("Renderer: strip " + String(tft.width()) + "x" + String(STRIP_H) +
                             " 16-bit, buffer=" + String(bufSize) + " bytes");
    return true;
}

inline bool reinitRenderer()
{
    if (!stripBuffer())
        return false;

    auto &c = canvas();
    // Reconfigure sprite for new screen width after rotation
    c.setBuffer(stripBuffer(), tft.width(), STRIP_H, 16);

    if (loggerInstance)
        loggerInstance->Info("Renderer reinit: strip " + String(tft.width()) + "x" + String(STRIP_H) +
                             " (buffer reused)");
    return true;
}

template <typename DrawFn> inline void renderStrips(DrawFn drawFn)
{
    auto &c = canvas();
    int sw = tft.width();
    int sh = tft.height();

    tft.startWrite();

    for (int sy = 0; sy < sh; sy += STRIP_H)
    {
        int stripH = ((sy + STRIP_H) > sh) ? (sh - sy) : STRIP_H;

        stripOffsetY() = sy;

        // Clear the strip buffer
        memset(stripBuffer(), 0, sw * stripH * 2);

        // Clip drawing to this strip's height
        c.setClipRect(0, 0, sw, stripH);

        drawFn();

        c.clearClipRect();

        // Push strip to screen. Use pushImage for partial height strips.
        tft.pushImage(0, sy, sw, stripH, (uint16_t *)stripBuffer());
    }

    stripOffsetY() = 0;
    tft.endWrite();
}

inline void flush()
{
    // no-op: renderStrips pushes each strip
}

namespace detail
{
inline bool &dirtyFlag()
{
    static bool flag = true;
    return flag;
}
} // namespace detail

inline void markDirty()
{
    detail::dirtyFlag() = true;
}
inline bool isDirty()
{
    return detail::dirtyFlag();
}
inline void clearDirty()
{
    detail::dirtyFlag() = false;
}

} // namespace UI
