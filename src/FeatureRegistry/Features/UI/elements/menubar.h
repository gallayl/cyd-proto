#pragma once

#include <functional>
#include <vector>
#include <cstring>
#include "container.h"
#include "menuitem.h"
#include "../Renderer.h"
#include "../Theme.h"

namespace UI
{

struct MenuBarEntry
{
    const char *label;
    std::vector<MenuItem> items;
};

class MenuBar : public Element
{
public:
    MenuBar() = default;

    void setMenus(std::vector<MenuBarEntry> menus)
    {
        _menus = std::move(menus);
    }

    bool isDropdownOpen() const
    {
        return openMenuIdx >= 0;
    }

    void closeDropdown()
    {
        openMenuIdx = -1;
        dropHighlight = -1;
    }

    void draw() override
    {
        if (!mounted)
            return;
        drawBar();
        if (openMenuIdx >= 0)
            drawDropdown();
    }

    void onTouch(int px, int py) override
    {
        if (!mounted)
            return;

        // check dropdown first
        if (openMenuIdx >= 0 && isInsideDropdown(px, py))
        {
            int idx = dropdownItemAt(py);
            dropHighlight = idx;
            return;
        }

        // check bar
        int idx = menuBarItemAt(px, py);
        if (idx >= 0)
        {
            if (openMenuIdx == idx)
                openMenuIdx = -1;
            else
                openMenuIdx = idx;
            dropHighlight = -1;
            return;
        }

        // outside — close
        if (openMenuIdx >= 0)
        {
            closeDropdown();
        }
    }

    void onTouchEnd(int px, int py) override
    {
        if (!mounted)
            return;

        if (openMenuIdx >= 0 && isInsideDropdown(px, py))
        {
            int idx = dropdownItemAt(py);
            auto &items = _menus[openMenuIdx].items;
            if (idx >= 0 && idx < (int)items.size() && items[idx].isLeaf())
            {
                auto action = items[idx].action;
                closeDropdown();
                if (action)
                    action();
                return;
            }
        }

        dropHighlight = -1;
    }

private:
    std::vector<MenuBarEntry> _menus;
    int openMenuIdx{-1};
    int dropHighlight{-1};

    void drawBar()
    {
        auto &c = canvas();

        c.fillRect(drawX(), drawY(), width, Theme::MenuBarHeight, Theme::MenuBg);
        c.drawFastHLine(drawX(), drawY() + Theme::MenuBarHeight - 1, width, Theme::ButtonShadow);

        c.setTextSize(1);
        int mx = drawX() + 4;
        for (int i = 0; i < (int)_menus.size(); i++)
        {
            int16_t tw = c.textWidth(_menus[i].label);
            int itemW = tw + 12;

            bool active = (i == openMenuIdx);
            if (active)
            {
                c.fillRect(mx, drawY(), itemW, Theme::MenuBarHeight - 1, Theme::MenuHighlight);
                c.setTextColor(Theme::MenuHighlightText, Theme::MenuHighlight);
            }
            else
            {
                c.setTextColor(Theme::TextColor, Theme::MenuBg);
            }

            int16_t th = c.fontHeight();
            c.setCursor(mx + 6, drawY() + (Theme::MenuBarHeight - th) / 2);
            c.print(_menus[i].label);
            mx += itemW;
        }
    }

    void drawDropdown()
    {
        if (openMenuIdx < 0 || openMenuIdx >= (int)_menus.size())
            return;

        auto &c = canvas();
        auto &items = _menus[openMenuIdx].items;

        int dx, dy;
        dropdownPos(dx, dy);
        dx = dx - x + drawX();
        dy = dy - y + drawY();
        int dw = Theme::MenuWidth;
        int dh = dropdownHeight();

        // panel
        c.fillRect(dx, dy, dw, dh, Theme::MenuBg);
        c.drawRect(dx, dy, dw, dh, Theme::WindowBorder);
        c.drawFastHLine(dx + 1, dy + 1, dw - 2, Theme::ButtonHighlight);
        c.drawFastVLine(dx + 1, dy + 1, dh - 2, Theme::ButtonHighlight);
        c.drawFastHLine(dx + 1, dy + dh - 2, dw - 2, Theme::ButtonShadow);
        c.drawFastVLine(dx + dw - 2, dy + 1, dh - 2, Theme::ButtonShadow);

        int curY = dy + 2;
        c.setTextSize(1);
        for (int i = 0; i < (int)items.size(); i++)
        {
            if (items[i].isSeparator())
            {
                int sepY = curY + Theme::MenuSeparatorHeight / 2 - 1;
                c.drawFastHLine(dx + 4, sepY, dw - 8, Theme::MenuSeparatorDark);
                c.drawFastHLine(dx + 4, sepY + 1, dw - 8, Theme::MenuSeparatorLight);
                curY += Theme::MenuSeparatorHeight;
                continue;
            }

            bool hl = (i == dropHighlight);
            if (hl)
            {
                c.fillRect(dx + 2, curY, dw - 4, Theme::MenuItemHeight, Theme::MenuHighlight);
                c.setTextColor(Theme::MenuHighlightText, Theme::MenuHighlight);
            }
            else
            {
                c.setTextColor(Theme::TextColor, Theme::MenuBg);
            }

            int16_t th = c.fontHeight();
            c.setCursor(dx + 6, curY + (Theme::MenuItemHeight - th) / 2);
            c.print(items[i].label);
            curY += Theme::MenuItemHeight;
        }
    }

    void dropdownPos(int &dx, int &dy) const
    {
        dx = x + 4;
        for (int i = 0; i < openMenuIdx; i++)
        {
            // approximate: we don't have canvas access here for textWidth,
            // so use a fixed width per character estimate
            int labelLen = strlen(_menus[i].label);
            dx += labelLen * 6 + 12;
        }
        dy = y + Theme::MenuBarHeight;
    }

    int dropdownHeight() const
    {
        if (openMenuIdx < 0 || openMenuIdx >= (int)_menus.size())
            return 0;
        int h = 4;
        for (auto &item : _menus[openMenuIdx].items)
        {
            h += item.isSeparator() ? Theme::MenuSeparatorHeight : Theme::MenuItemHeight;
        }
        return h;
    }

    int menuBarItemAt(int px, int py) const
    {
        if (py < y || py >= y + Theme::MenuBarHeight)
            return -1;
        int mx = x + 4;
        for (int i = 0; i < (int)_menus.size(); i++)
        {
            int labelLen = strlen(_menus[i].label);
            int itemW = labelLen * 6 + 12;
            if (px >= mx && px < mx + itemW)
                return i;
            mx += itemW;
        }
        return -1;
    }

    bool isInsideDropdown(int px, int py) const
    {
        if (openMenuIdx < 0)
            return false;
        int dx, dy;
        const_cast<MenuBar *>(this)->dropdownPos(dx, dy);
        int dw = Theme::MenuWidth;
        int dh = const_cast<MenuBar *>(this)->dropdownHeight();
        return px >= dx && px < dx + dw && py >= dy && py < dy + dh;
    }

    int dropdownItemAt(int py) const
    {
        if (openMenuIdx < 0 || openMenuIdx >= (int)_menus.size())
            return -1;
        int dx, dy;
        const_cast<MenuBar *>(this)->dropdownPos(dx, dy);
        int relY = py - dy - 2;
        if (relY < 0)
            return -1;
        int accum = 0;
        auto &items = _menus[openMenuIdx].items;
        for (int i = 0; i < (int)items.size(); i++)
        {
            int itemH = items[i].isSeparator() ? Theme::MenuSeparatorHeight : Theme::MenuItemHeight;
            if (relY < accum + itemH)
            {
                if (items[i].isSeparator())
                    return -1;
                return i;
            }
            accum += itemH;
        }
        return -1;
    }
};

} // namespace UI
