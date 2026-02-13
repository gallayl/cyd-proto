#pragma once

#include <LovyanGFX.hpp>
#include <Arduino.h>
#include "../../../../hw/Screen.h"
#include "container.h" // provides UI::Element base

// display object from hardware layer
extern LGFX tft;

namespace UI
{

    // simple text label; will draw itself when mounted and whenever the
    // text changes. positioning is controlled via setBounds inherited from
    // Element.
    class Label : public Element
    {
    public:
        Label(const String &txt = String(), int ix = 0, int iy = 0, int iw = 0, int ih = 0)
            : text(txt)
        {
            setBounds(ix, iy, iw, ih);
        }

        void setText(const String &txt)
        {
            text = txt;
            if (mounted)
                draw();
        }

        void setTextColor(uint16_t fg, uint16_t bg = TFT_BLACK)
        {
            color = fg;
            bgColor = bg;
        }

        void setTextSize(uint8_t size) { textSize = size; }

        void draw() override
        {
            if (!mounted)
                return;
            tft.setTextColor(color, bgColor);
            tft.setTextSize(textSize);

            // try to center the text inside our bounds; falls back to
            // top-left if measuring APIs aren't available.
            int16_t tx = x;
            int16_t ty = y;
            if (width > 0 && height > 0) {
                // textWidth/fontHeight automatically respect the current
                // text size/font, so we don't pass our stored textSize.
                int16_t tw = tft.textWidth(text);
                int16_t th = tft.fontHeight();
                if (tw < width)
                    tx = x + (width - tw) / 2;
                if (th < height)
                    ty = y + (height - th) / 2;
            }

            tft.setCursor(tx, ty);
            tft.print(text);
        }

    private:
        String text;
        uint16_t color{TFT_WHITE};
        uint16_t bgColor{TFT_BLACK};
        uint8_t textSize{1};
    };

} // namespace UI
