#pragma once

#include <LovyanGFX.hpp>
#include "label.h"
#include "../ActionQueue.h"
#include "../Renderer.h"

namespace UI
{

// a basic rectangular button with a label. emits a callback when a
// press is detected (touch down + up within bounds).
class Button : public Element
{
public:
    using Callback = std::function<void()>;

    Button(const std::string &txt = "", int ix = 0, int iy = 0, int iw = 0, int ih = 0) : label(txt, ix, iy, iw, ih)
    {
        setBounds(ix, iy, iw, ih);
        labelFg = TFT_BLACK;
        label.setTextColor(labelFg, bgColor);
    }

    void setCallback(Callback cb)
    {
        onClick = std::move(cb);
    }
    void setLabel(const std::string &txt)
    {
        label.setText(txt);
    }

    void setBackgroundColor(uint16_t c)
    {
        bgColor = c;
        label.setTextColor(labelFg, bgColor);
    }
    void setTextColor(uint16_t fg)
    {
        labelFg = fg;
        label.setTextColor(fg, bgColor);
    }
    void setTextColor(uint16_t fg, uint16_t bg)
    {
        labelFg = fg;
        label.setTextColor(fg, bg);
    }
    void setTextSize(uint8_t size)
    {
        label.setTextSize(size);
    }
    void setTextAlign(TextAlign a)
    {
        label.setAlign(a);
    }

    bool isPressed() const
    {
        return pressed;
    }
    void setPressed(bool p)
    {
        pressed = p;
    }

    void draw() override
    {
        if (!mounted)
            return;
        auto &c = canvas();
        int dx = drawX(), dy = drawY();

        uint16_t light = pressed ? borderDarkColor : borderLightColor;
        uint16_t dark = pressed ? borderLightColor : borderDarkColor;

        c.fillRect(dx, dy, width, height, bgColor);

        // top/left light edge
        c.drawFastHLine(dx, dy, width, light);
        c.drawFastVLine(dx, dy, height, light);
        // bottom/right dark edge
        c.drawFastHLine(dx, dy + height - 1, width, dark);
        c.drawFastVLine(dx + width - 1, dy, height, dark);

        if (pressed)
        {
            label.setBounds(x + 1, y + 1, width - 1, height - 1);
        }
        else
        {
            label.setBounds(x, y, width, height);
        }
        label.draw();
    }

    void onTouch(int px, int py) override
    {
        if (contains(px, py))
        {
            pressed = true;
        }
    }

    void onTouchEnd(int px, int py) override
    {
        if (pressed && contains(px, py) && onClick)
        {
            UI::queueAction(onClick);
        }
        pressed = false;
    }

    void mount() override
    {
        Element::mount();
        label.mount();
    }

    void setBorderColors(uint16_t light, uint16_t dark)
    {
        borderLightColor = light;
        borderDarkColor = dark;
    }

    void setBounds(int ix, int iy, int iw, int ih)
    {
        Element::setBounds(ix, iy, iw, ih);
        label.setBounds(ix, iy, iw, ih);
    }

private:
    Label label;
    Callback onClick;
    bool pressed{false};
    uint16_t bgColor{TFT_LIGHTGREY};
    uint16_t labelFg{TFT_WHITE};
    uint16_t borderLightColor{TFT_WHITE};
    uint16_t borderDarkColor{TFT_BLACK};
};

} // namespace UI
