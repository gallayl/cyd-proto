#pragma once

#include <functional>
#include <string>
#include "container.h"
#include "../Renderer.h"
#include "../Theme.h"
#include "../ActionQueue.h"

namespace UI
{

class Checkbox : public Element
{
public:
    using ChangeCb = std::function<void(bool)>;

    Checkbox(const std::string &txt = "", int ix = 0, int iy = 0, int iw = 0, int ih = 0) : labelText(txt)
    {
        setBounds(ix, iy, iw, ih);
    }

    void setChecked(bool c)
    {
        checked = c;
    }
    bool isChecked() const
    {
        return checked;
    }

    void setLabel(const std::string &txt)
    {
        labelText = txt;
    }
    void setTextColor(uint16_t fg)
    {
        textFg = fg;
    }
    void setOnChange(ChangeCb cb)
    {
        onChange = std::move(cb);
    }

    void draw() override
    {
        if (!mounted)
            return;
        auto &c = canvas();

        int boxSize = Theme::CheckboxSize;
        int boxX = drawX() + 2;
        int boxY = drawY() + (height - boxSize) / 2;

        // sunken 3D box (dark outer top-left, light inner bottom-right)
        c.fillRect(boxX, boxY, boxSize, boxSize, Theme::CheckboxBg);
        c.drawFastHLine(boxX, boxY, boxSize, Theme::ButtonShadow);
        c.drawFastVLine(boxX, boxY, boxSize, Theme::ButtonShadow);
        c.drawFastHLine(boxX + 1, boxY + 1, boxSize - 2, Theme::ButtonDarkShadow);
        c.drawFastVLine(boxX + 1, boxY + 1, boxSize - 2, Theme::ButtonDarkShadow);
        c.drawFastHLine(boxX, boxY + boxSize - 1, boxSize, Theme::ButtonHighlight);
        c.drawFastVLine(boxX + boxSize - 1, boxY, boxSize, Theme::ButtonHighlight);
        c.drawFastHLine(boxX + 1, boxY + boxSize - 2, boxSize - 2, Theme::ButtonFace);
        c.drawFastVLine(boxX + boxSize - 2, boxY + 1, boxSize - 2, Theme::ButtonFace);

        // checkmark
        if (checked)
        {
            int cx = boxX + 3;
            int cy = boxY + boxSize / 2;
            c.drawLine(cx, cy, cx + 2, cy + 2, Theme::TextColor);
            c.drawLine(cx + 2, cy + 2, cx + 7, cy - 3, Theme::TextColor);
            c.drawLine(cx, cy + 1, cx + 2, cy + 3, Theme::TextColor);
            c.drawLine(cx + 2, cy + 3, cx + 7, cy - 2, Theme::TextColor);
        }

        // label text
        if (labelText.length() > 0)
        {
            int labelX = boxX + boxSize + 4;
            c.setTextColor(textFg, Theme::WindowBg);
            c.setTextSize(1);
            int16_t th = c.fontHeight();
            int labelY = drawY() + (height - th) / 2;
            c.setCursor(labelX, labelY);
            c.print(labelText.c_str());
        }
    }

    void onTouch(int px, int py) override
    {
        if (contains(px, py))
            pressing = true;
    }

    void onTouchEnd(int px, int py) override
    {
        if (pressing && contains(px, py))
        {
            checked = !checked;
            if (onChange)
                UI::queueAction([this]() { onChange(checked); });
        }
        pressing = false;
    }

private:
    std::string labelText;
    bool checked{false};
    bool pressing{false};
    uint16_t textFg{TFT_BLACK};
    ChangeCb onChange;
};

} // namespace UI
