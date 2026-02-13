#pragma once

#include <functional>
#include <vector>
#include "../Renderer.h"
#include "../Theme.h"
#include "../App.h"

namespace UI
{

    class StartMenu
    {
    public:
        using AppSelectedCb = std::function<void(const char *)>;

        void setAppSelectedCallback(AppSelectedCb cb) { onAppSelected = std::move(cb); }

        bool isVisible() const { return visible; }

        void toggle()
        {
            visible = !visible;
            highlightIndex = -1;
        }

        void show()
        {
            visible = true;
            highlightIndex = -1;
        }
        void hide()
        {
            visible = false;
            highlightIndex = -1;
        }

        void draw()
        {
            if (!visible)
                return;

            auto &c = canvas();
            auto &reg = appRegistry();
            int itemCount = (int)reg.size();
            if (itemCount == 0)
                return;

            int menuW = 120;
            int menuH = itemCount * Theme::MenuItemHeight + 4;
            int menuX = 2;
            int menuY = Theme::TaskbarY - menuH;

            // outer border raised 3D
            c.fillRect(menuX, menuY, menuW, menuH, Theme::MenuBg);
            c.drawRect(menuX, menuY, menuW, menuH, Theme::WindowBorder);
            c.drawFastHLine(menuX + 1, menuY + 1, menuW - 2, Theme::ButtonHighlight);
            c.drawFastVLine(menuX + 1, menuY + 1, menuH - 2, Theme::ButtonHighlight);
            c.drawFastHLine(menuX + 1, menuY + menuH - 2, menuW - 2, Theme::ButtonShadow);
            c.drawFastVLine(menuX + menuW - 2, menuY + 1, menuH - 2, Theme::ButtonShadow);

            // menu items
            for (int i = 0; i < itemCount; i++)
            {
                int iy = menuY + 2 + i * Theme::MenuItemHeight;
                bool hl = (i == highlightIndex);
                if (hl)
                {
                    c.fillRect(menuX + 2, iy, menuW - 4, Theme::MenuItemHeight, Theme::MenuHighlight);
                    c.setTextColor(Theme::MenuHighlightText, Theme::MenuHighlight);
                }
                else
                {
                    c.setTextColor(Theme::TextColor, Theme::MenuBg);
                }
                c.setTextSize(1);
                int16_t th = c.fontHeight();
                int tx = menuX + 6;
                int ty = iy + (Theme::MenuItemHeight - th) / 2;
                c.setCursor(tx, ty);
                c.print(reg[i].name);
            }

            menuBounds[0] = menuX;
            menuBounds[1] = menuY;
            menuBounds[2] = menuW;
            menuBounds[3] = menuH;
        }

        bool handleTouch(int px, int py)
        {
            if (!visible)
                return false;

            if (px >= menuBounds[0] && px < menuBounds[0] + menuBounds[2] &&
                py >= menuBounds[1] && py < menuBounds[1] + menuBounds[3])
            {
                int relY = py - menuBounds[1] - 2;
                if (relY >= 0)
                {
                    highlightIndex = relY / Theme::MenuItemHeight;
                    auto &reg = appRegistry();
                    if (highlightIndex >= (int)reg.size())
                        highlightIndex = -1;
                }
                return true;
            }

            // touch outside menu closes it
            hide();
            return false;
        }

        bool handleTouchEnd(int px, int py)
        {
            if (!visible)
                return false;

            if (px >= menuBounds[0] && px < menuBounds[0] + menuBounds[2] &&
                py >= menuBounds[1] && py < menuBounds[1] + menuBounds[3])
            {
                int relY = py - menuBounds[1] - 2;
                if (relY >= 0)
                {
                    int idx = relY / Theme::MenuItemHeight;
                    auto &reg = appRegistry();
                    if (idx >= 0 && idx < (int)reg.size())
                    {
                        if (onAppSelected)
                            onAppSelected(reg[idx].name);
                        hide();
                        return true;
                    }
                }
            }

            hide();
            return false;
        }

    private:
        AppSelectedCb onAppSelected;
        bool visible{false};
        int highlightIndex{-1};
        int menuBounds[4]{0, 0, 0, 0}; // x, y, w, h
    };

} // namespace UI
