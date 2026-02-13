#pragma once

#include <LovyanGFX.hpp>
#include <Arduino.h>
#include "container.h"
#include "../Renderer.h"

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
        }

        const String &getText() const { return text; }

        void setTextColor(uint16_t fg, uint16_t bg = TFT_BLACK)
        {
            color = fg;
            bgColor = bg;
        }

        void setTextSize(uint8_t size) { textSize = size; }

        void setAlign(lgfx::textdatum_t datum) { align = datum; }

        void draw() override
        {
            if (!mounted)
                return;
            auto &c = canvas();
            c.setTextColor(color, bgColor);
            c.setTextSize(textSize);

            // try to center the text inside our bounds; falls back to
            // top-left if measuring APIs aren't available.
            int16_t tx = x;
            int16_t ty = y;
            if (width > 0 && height > 0)
            {
                int16_t tw = c.textWidth(text);
                int16_t th = c.fontHeight();
                if (align == lgfx::textdatum_t::top_left)
                {
                    if (tw < width)
                        tx = x + (width - tw) / 2;
                }
                else if (align == lgfx::textdatum_t::top_right)
                {
                    tx = x + width - tw - 2;
                }
                if (th < height)
                    ty = y + (height - th) / 2;
            }

            c.setCursor(tx, ty);
            c.print(text);
        }

    private:
        String text;
        uint16_t color{TFT_WHITE};
        uint16_t bgColor{TFT_BLACK};
        uint8_t textSize{1};
        lgfx::textdatum_t align{lgfx::textdatum_t::top_left};
    };

} // namespace UI
