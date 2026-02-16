#pragma once

#include <string>
#include <LovyanGFX.hpp>
#include "container.h"
#include "../Renderer.h"

namespace UI
{

enum class TextAlign
{
    CENTER,
    LEFT,
    RIGHT
};

// simple text label; will draw itself when mounted and whenever the
// text changes. positioning is controlled via setBounds inherited from
// Element.
class Label : public Element
{
public:
    Label(const std::string &txt = "", int ix = 0, int iy = 0, int iw = 0, int ih = 0) : text(txt)
    {
        setBounds(ix, iy, iw, ih);
    }

    void setText(const std::string &txt)
    {
        text = txt;
    }

    const std::string &getText() const
    {
        return text;
    }

    void setTextColor(uint16_t fg, uint16_t bg = TFT_BLACK)
    {
        color = fg;
        bgColor = bg;
    }

    void setTextSize(uint8_t size)
    {
        textSize = size;
    }

    void setAlign(TextAlign a)
    {
        align = a;
    }

    void draw() override
    {
        if (!mounted)
            return;
        auto &c = canvas();
        c.setTextColor(color, bgColor);
        c.setTextSize(textSize);

        int16_t tx = drawX();
        int16_t ty = drawY();
        if (width > 0 && height > 0)
        {
            int16_t tw = c.textWidth(text.c_str());
            int16_t th = c.fontHeight();

            switch (align)
            {
            case TextAlign::CENTER:
                if (tw < width)
                    tx = drawX() + (width - tw) / 2;
                break;
            case TextAlign::LEFT:
                tx = drawX() + 2;
                break;
            case TextAlign::RIGHT:
                tx = drawX() + width - tw - 2;
                break;
            }

            if (th < height)
                ty = drawY() + (height - th) / 2;
        }

        c.setCursor(tx, ty);
        c.print(text.c_str());
    }

private:
    std::string text;
    uint16_t color{TFT_WHITE};
    uint16_t bgColor{TFT_BLACK};
    uint8_t textSize{1};
    TextAlign align{TextAlign::CENTER};
};

} // namespace UI
