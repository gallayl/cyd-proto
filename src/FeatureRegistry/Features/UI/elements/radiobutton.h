#pragma once

#include <functional>
#include <string>
#include <vector>
#include "container.h"
#include "../Renderer.h"
#include "../Theme.h"
#include "../ActionQueue.h"

namespace UI
{

class RadioButton;

class RadioGroup
{
public:
    void addButton(RadioButton *btn)
    {
        buttons.push_back(btn);
    }
    void selectButton(RadioButton *btn);
    RadioButton *getSelected() const
    {
        return selected;
    }

private:
    std::vector<RadioButton *> buttons;
    RadioButton *selected{nullptr};
};

class RadioButton : public Element
{
public:
    using ChangeCb = std::function<void(bool)>;

    RadioButton(const std::string &txt = "", int ix = 0, int iy = 0, int iw = 0, int ih = 0) : labelText(txt)
    {
        setBounds(ix, iy, iw, ih);
    }

    void setSelected(bool s)
    {
        selected = s;
    }
    bool isSelected() const
    {
        return selected;
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
    void setGroup(RadioGroup *g)
    {
        group = g;
        if (g)
            g->addButton(this);
    }

    void draw() override
    {
        if (!mounted)
            return;
        auto &c = canvas();

        int r = Theme::RadioSize / 2;
        int cx = drawX() + 2 + r;
        int cy = drawY() + height / 2;

        // outer circle background
        c.fillCircle(cx, cy, r, Theme::CheckboxBg);

        // 3D circle border — dark top-left arc, light bottom-right
        c.drawCircle(cx, cy, r, Theme::ButtonShadow);
        // small highlight arc at bottom-right
        c.drawArc(cx, cy, r, r, 315, 135, Theme::ButtonHighlight);
        c.drawArc(cx, cy, r, r, 135, 315, Theme::ButtonDarkShadow);

        // dot when selected
        if (selected)
        {
            c.fillCircle(cx, cy, r - 3, Theme::TextColor);
        }

        // label text
        if (labelText.length() > 0)
        {
            int labelX = drawX() + 2 + Theme::RadioSize + 4;
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
            if (!selected)
            {
                if (group)
                    group->selectButton(this);
                else
                    selected = true;
                if (onChange)
                    UI::queueAction([this]() { onChange(selected); });
            }
        }
        pressing = false;
    }

private:
    std::string labelText;
    bool selected{false};
    bool pressing{false};
    uint16_t textFg{TFT_BLACK};
    ChangeCb onChange;
    RadioGroup *group{nullptr};
};

inline void RadioGroup::selectButton(RadioButton *btn)
{
    if (selected == btn)
        return;
    if (selected)
        selected->setSelected(false);
    selected = btn;
    if (btn)
        btn->setSelected(true);
}

} // namespace UI
