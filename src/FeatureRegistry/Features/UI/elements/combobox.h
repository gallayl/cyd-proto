#pragma once

#include <functional>
#include <vector>
#include "container.h"
#include "../Renderer.h"
#include "../Theme.h"
#include "../ActionQueue.h"

namespace UI
{

class ComboBox : public Element
{
public:
    using ChangeCb = std::function<void(int)>;

    ComboBox(int ix = 0, int iy = 0, int iw = 0, int ih = 0)
    {
        setBounds(ix, iy, iw, ih);
    }

    void setItems(const std::vector<String> &items)
    {
        _items = items;
    }
    void addItem(const String &item)
    {
        _items.push_back(item);
    }
    void clearItems()
    {
        _items.clear();
        selectedIndex = -1;
    }

    void setSelectedIndex(int idx)
    {
        if (idx >= -1 && idx < (int)_items.size())
            selectedIndex = idx;
    }
    int getSelectedIndex() const
    {
        return selectedIndex;
    }
    String getSelectedText() const
    {
        if (selectedIndex >= 0 && selectedIndex < (int)_items.size())
            return _items[selectedIndex];
        return String();
    }

    void setOnChange(ChangeCb cb)
    {
        onChange = std::move(cb);
    }

    bool isOpen() const
    {
        return open;
    }

    void draw() override
    {
        if (!mounted)
            return;
        auto &c = canvas();

        // main button area
        c.fillRect(drawX(), drawY(), width, height, Theme::TextFieldBg);

        // sunken border
        c.drawFastHLine(drawX(), drawY(), width, Theme::ButtonShadow);
        c.drawFastVLine(drawX(), drawY(), height, Theme::ButtonShadow);
        c.drawFastHLine(drawX() + 1, drawY() + 1, width - 2, Theme::ButtonDarkShadow);
        c.drawFastVLine(drawX() + 1, drawY() + 1, height - 2, Theme::ButtonDarkShadow);
        c.drawFastHLine(drawX(), drawY() + height - 1, width, Theme::ButtonHighlight);
        c.drawFastVLine(drawX() + width - 1, drawY(), height, Theme::ButtonHighlight);

        // dropdown arrow button
        int arrowW = height;
        int arrowX = drawX() + width - arrowW;
        c.fillRect(arrowX, drawY() + 1, arrowW - 1, height - 2, Theme::ButtonFace);
        c.drawFastHLine(arrowX, drawY() + 1, arrowW - 1, Theme::ButtonHighlight);
        c.drawFastVLine(arrowX, drawY() + 1, height - 2, Theme::ButtonHighlight);
        c.drawFastHLine(arrowX, drawY() + height - 2, arrowW - 1, Theme::ButtonShadow);
        c.drawFastVLine(arrowX + arrowW - 2, drawY() + 1, height - 2, Theme::ButtonShadow);

        // arrow triangle
        int aCx = arrowX + arrowW / 2;
        int aCy = drawY() + height / 2;
        c.fillTriangle(aCx - 3, aCy - 2, aCx + 3, aCy - 2, aCx, aCy + 2, Theme::TextColor);

        // selected text
        c.setTextSize(1);
        if (selectedIndex >= 0 && selectedIndex < (int)_items.size())
        {
            c.setTextColor(Theme::TextColor, Theme::TextFieldBg);
            int16_t th = c.fontHeight();
            c.setClipRect(drawX() + 3, drawY() + 2, width - arrowW - 4, height - 4);
            c.setCursor(drawX() + 3, drawY() + (height - th) / 2);
            c.print(_items[selectedIndex]);
            c.clearClipRect();
        }

        // dropdown list
        if (open)
            drawDropdown(c);
    }

    void onTouch(int px, int py) override
    {
        if (!mounted)
            return;

        if (open)
        {
            int dropY = y + height;
            int dropH = dropdownHeight();
            if (py >= dropY && py < dropY + dropH && px >= x && px < x + width)
            {
                int itemH = Theme::MenuItemHeight;
                int idx = (py - dropY) / itemH;
                if (idx >= 0 && idx < (int)_items.size())
                    highlightIdx = idx;
            }
            return;
        }

        if (contains(px, py))
            pressing = true;
    }

    void onTouchEnd(int px, int py) override
    {
        if (!mounted)
            return;

        if (open)
        {
            int dropY = y + height;
            int dropH = dropdownHeight();
            if (py >= dropY && py < dropY + dropH && px >= x && px < x + width)
            {
                int itemH = Theme::MenuItemHeight;
                int idx = (py - dropY) / itemH;
                if (idx >= 0 && idx < (int)_items.size())
                {
                    selectedIndex = idx;
                    if (onChange)
                        UI::queueAction([this]() { onChange(selectedIndex); });
                }
            }
            open = false;
            highlightIdx = -1;
            return;
        }

        if (pressing && contains(px, py))
        {
            open = !open;
            highlightIdx = -1;
        }
        pressing = false;
    }

private:
    std::vector<String> _items;
    int selectedIndex{-1};
    bool open{false};
    bool pressing{false};
    int highlightIdx{-1};
    ChangeCb onChange;

    int dropdownHeight() const
    {
        int maxVisible = 6;
        int count = (int)_items.size();
        if (count > maxVisible)
            count = maxVisible;
        return count * Theme::MenuItemHeight + 4;
    }

    void drawDropdown(LGFX_Sprite &c)
    {
        int dropX = drawX();
        int dropY = drawY() + height;
        int dropW = width;
        int dropH = dropdownHeight();

        // border
        c.fillRect(dropX, dropY, dropW, dropH, Theme::TextFieldBg);
        c.drawRect(dropX, dropY, dropW, dropH, Theme::WindowBorder);

        int itemH = Theme::MenuItemHeight;
        int curY = dropY + 2;
        int maxVisible = 6;
        int count = (int)_items.size();
        if (count > maxVisible)
            count = maxVisible;

        c.setTextSize(1);
        for (int i = 0; i < count; i++)
        {
            bool hl = (i == highlightIdx);
            if (hl)
            {
                c.fillRect(dropX + 1, curY, dropW - 2, itemH, Theme::MenuHighlight);
                c.setTextColor(Theme::MenuHighlightText, Theme::MenuHighlight);
            }
            else
            {
                c.setTextColor(Theme::TextColor, Theme::TextFieldBg);
            }
            int16_t th = c.fontHeight();
            c.setCursor(dropX + 4, curY + (itemH - th) / 2);
            c.print(_items[i]);
            curY += itemH;
        }
    }
};

} // namespace UI
