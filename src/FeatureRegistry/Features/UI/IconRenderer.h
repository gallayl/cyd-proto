#pragma once

#include <LovyanGFX.hpp>

namespace UI
{

enum class IconSize : uint8_t
{
    Icon16x16 = 16,
    Icon32x32 = 32
};

// 4-bit indexed color palette matching classic Windows 95/3.1 system colors (RGB565)
namespace IconPalette
{
constexpr uint16_t colors[16] = {
    0x0000, // 0  Black
    0x0010, // 1  Navy (dark blue)
    0x0400, // 2  Dark green
    0x0410, // 3  Teal
    0x8000, // 4  Dark red
    0x8010, // 5  Purple
    0x8400, // 6  Olive/brown
    0xC618, // 7  Light gray (ButtonFace / Silver)
    0x8410, // 8  Dark gray
    0x001F, // 9  Blue
    0x07E0, // 10 Green
    0x07FF, // 11 Cyan
    0xF800, // 12 Red
    0xF81F, // 13 Magenta
    0xFFE0, // 14 Yellow
    0xFFFF, // 15 White
};
} // namespace IconPalette

struct IconData
{
    uint8_t width;
    uint8_t height;
    const uint8_t *data; // 4-bit packed pixels (2 pixels per byte, high nibble first)
};

// Draw a 4-bit indexed icon from PROGMEM at the given position
inline void drawIndexedIcon(LGFX_Sprite &canvas, const IconData &icon, int x, int y)
{
    for (int row = 0; row < icon.height; row++)
    {
        for (int col = 0; col < icon.width; col++)
        {
            int pixelIndex = (icon.width * row) + col;
            int byteIndex = pixelIndex / 2;
            uint8_t packed = pgm_read_byte(&icon.data[byteIndex]);
            uint8_t colorIdx;
            if (pixelIndex % 2 == 0)
            {
                colorIdx = (packed >> 4) & 0x0F; // high nibble
            }
            else
            {
                colorIdx = packed & 0x0F; // low nibble
            }

            if (colorIdx == 0)
            {
                continue; // treat palette index 0 (black) as transparent for icons
            }

            canvas.drawPixel(x + col, y + row, IconPalette::colors[colorIdx]);
        }
    }
}

// Draw an icon scaled to a target size (nearest-neighbor scaling)
inline void drawIndexedIconScaled(LGFX_Sprite &canvas, const IconData &icon, int x, int y, int targetSize)
{
    if (targetSize == icon.width && targetSize == icon.height)
    {
        drawIndexedIcon(canvas, icon, x, y);
        return;
    }

    for (int ty = 0; ty < targetSize; ty++)
    {
        int srcY = ty * icon.height / targetSize;
        for (int tx = 0; tx < targetSize; tx++)
        {
            int srcX = tx * icon.width / targetSize;
            int pixelIndex = srcY * icon.width + srcX;
            int byteIndex = pixelIndex / 2;
            uint8_t packed = pgm_read_byte(&icon.data[byteIndex]);
            uint8_t colorIdx;
            if (pixelIndex % 2 == 0)
                colorIdx = (packed >> 4) & 0x0F;
            else
                colorIdx = packed & 0x0F;

            if (colorIdx == 0)
                continue;

            canvas.drawPixel(x + tx, y + ty, IconPalette::colors[colorIdx]);
        }
    }
}

} // namespace UI
