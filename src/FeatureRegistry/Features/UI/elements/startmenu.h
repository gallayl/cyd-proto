#pragma once

#include <functional>
#include <vector>
#include "../Renderer.h"
#include "../Theme.h"

namespace UI
{

    struct MenuItem
    {
        const char *label;
        std::vector<MenuItem> children;
        std::function<void()> action;
        bool separator{false};

        bool hasSubmenu() const { return !children.empty(); }
        bool isLeaf() const { return !hasSubmenu() && action != nullptr; }
        bool isSeparator() const { return separator; }

        static MenuItem Leaf(const char *label, std::function<void()> action)
        {
            return MenuItem{label, {}, std::move(action), false};
        }

        static MenuItem Submenu(const char *label, std::vector<MenuItem> children)
        {
            return MenuItem{label, std::move(children), nullptr, false};
        }

        static MenuItem Separator()
        {
            return MenuItem{"", {}, nullptr, true};
        }
    };

    class StartMenu
    {
    public:
        using AppSelectedCb = std::function<void(const char *)>;

        void setAppSelectedCallback(AppSelectedCb cb) { onAppSelected = std::move(cb); }

        void setMenuItems(std::vector<MenuItem> items) { menuItems = std::move(items); }

        bool isVisible() const { return visible; }

        void toggle()
        {
            if (visible)
                hide();
            else
                show();
        }

        void show()
        {
            visible = true;
            highlightIndex = -1;
            subHighlightIndex = -1;
            activeSubmenu = -1;
        }

        void hide()
        {
            visible = false;
            highlightIndex = -1;
            subHighlightIndex = -1;
            activeSubmenu = -1;
        }

        void draw()
        {
            if (!visible)
                return;

            auto &c = canvas();
            int itemCount = (int)menuItems.size();
            if (itemCount == 0)
                return;

            int menuW = Theme::MenuWidth;
            int menuH = calcMenuHeight(menuItems);
            int menuX = 2;
            int menuY = Theme::TaskbarY - menuH;

            drawMenuPanel(c, menuX, menuY, menuW, menuH, menuItems, highlightIndex);

            mainBounds[0] = menuX;
            mainBounds[1] = menuY;
            mainBounds[2] = menuW;
            mainBounds[3] = menuH;

            // draw submenu if active
            if (activeSubmenu >= 0 && activeSubmenu < itemCount &&
                menuItems[activeSubmenu].hasSubmenu())
            {
                auto &sub = menuItems[activeSubmenu].children;
                int subW = Theme::SubMenuWidth;
                int subH = calcMenuHeight(sub);
                int subX = menuX + menuW - 1;
                int subY = menuY + getItemY(menuItems, activeSubmenu);
                // ensure submenu doesn't go above screen
                if (subY < 0)
                    subY = 0;
                // ensure submenu doesn't go below taskbar
                if (subY + subH > Theme::TaskbarY)
                    subY = Theme::TaskbarY - subH;

                drawMenuPanel(c, subX, subY, subW, subH, sub, subHighlightIndex);

                subBounds[0] = subX;
                subBounds[1] = subY;
                subBounds[2] = subW;
                subBounds[3] = subH;
            }
            else
            {
                subBounds[0] = subBounds[1] = subBounds[2] = subBounds[3] = 0;
            }
        }

        bool handleTouch(int px, int py)
        {
            if (!visible)
                return false;

            // check submenu first
            if (subBounds[2] > 0 && inBounds(px, py, subBounds))
            {
                int idx = getItemIndexAt(menuItems[activeSubmenu].children, py - subBounds[1] - 2);
                subHighlightIndex = idx;
                return true;
            }

            // check main menu
            if (inBounds(px, py, mainBounds))
            {
                int idx = getItemIndexAt(menuItems, py - mainBounds[1] - 2);
                highlightIndex = idx;

                if (idx >= 0 && idx < (int)menuItems.size() && menuItems[idx].hasSubmenu())
                {
                    activeSubmenu = idx;
                    subHighlightIndex = -1;
                }
                else
                {
                    activeSubmenu = -1;
                    subHighlightIndex = -1;
                }
                return true;
            }

            // touch outside closes menu
            hide();
            return false;
        }

        bool handleTouchEnd(int px, int py)
        {
            if (!visible)
                return false;

            // check submenu first
            if (subBounds[2] > 0 && inBounds(px, py, subBounds))
            {
                auto &sub = menuItems[activeSubmenu].children;
                int idx = getItemIndexAt(sub, py - subBounds[1] - 2);
                if (idx >= 0 && idx < (int)sub.size() && sub[idx].isLeaf())
                {
                    sub[idx].action();
                    hide();
                    return true;
                }
                return true;
            }

            // check main menu
            if (inBounds(px, py, mainBounds))
            {
                int idx = getItemIndexAt(menuItems, py - mainBounds[1] - 2);
                if (idx >= 0 && idx < (int)menuItems.size())
                {
                    if (menuItems[idx].isLeaf())
                    {
                        menuItems[idx].action();
                        hide();
                        return true;
                    }
                    if (menuItems[idx].hasSubmenu())
                    {
                        activeSubmenu = idx;
                        subHighlightIndex = -1;
                        return true;
                    }
                }
                return true;
            }

            hide();
            return false;
        }

    private:
        AppSelectedCb onAppSelected;
        std::vector<MenuItem> menuItems;
        bool visible{false};
        int highlightIndex{-1};
        int subHighlightIndex{-1};
        int activeSubmenu{-1};
        int mainBounds[4]{0, 0, 0, 0};
        int subBounds[4]{0, 0, 0, 0};

        static bool inBounds(int px, int py, const int b[4])
        {
            return px >= b[0] && px < b[0] + b[2] &&
                   py >= b[1] && py < b[1] + b[3];
        }

        static int calcMenuHeight(const std::vector<MenuItem> &items)
        {
            int h = 4; // 2px padding top + 2px padding bottom
            for (auto &item : items)
            {
                if (item.isSeparator())
                    h += Theme::MenuSeparatorHeight;
                else
                    h += Theme::MenuItemHeight;
            }
            return h;
        }

        static int getItemY(const std::vector<MenuItem> &items, int index)
        {
            int y = 2;
            for (int i = 0; i < index && i < (int)items.size(); i++)
            {
                if (items[i].isSeparator())
                    y += Theme::MenuSeparatorHeight;
                else
                    y += Theme::MenuItemHeight;
            }
            return y;
        }

        static int getItemIndexAt(const std::vector<MenuItem> &items, int relY)
        {
            if (relY < 0)
                return -1;
            int accum = 0;
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

        static void drawMenuPanel(LGFX_Sprite &c, int mx, int my, int mw, int mh,
                                  const std::vector<MenuItem> &items, int hlIndex)
        {
            // outer border raised 3D
            c.fillRect(mx, my, mw, mh, Theme::MenuBg);
            c.drawRect(mx, my, mw, mh, Theme::WindowBorder);
            c.drawFastHLine(mx + 1, my + 1, mw - 2, Theme::ButtonHighlight);
            c.drawFastVLine(mx + 1, my + 1, mh - 2, Theme::ButtonHighlight);
            c.drawFastHLine(mx + 1, my + mh - 2, mw - 2, Theme::ButtonShadow);
            c.drawFastVLine(mx + mw - 2, my + 1, mh - 2, Theme::ButtonShadow);

            int curY = my + 2;
            for (int i = 0; i < (int)items.size(); i++)
            {
                if (items[i].isSeparator())
                {
                    int sepY = curY + Theme::MenuSeparatorHeight / 2 - 1;
                    c.drawFastHLine(mx + 4, sepY, mw - 8, Theme::MenuSeparatorDark);
                    c.drawFastHLine(mx + 4, sepY + 1, mw - 8, Theme::MenuSeparatorLight);
                    curY += Theme::MenuSeparatorHeight;
                    continue;
                }

                bool hl = (i == hlIndex);
                if (hl)
                {
                    c.fillRect(mx + 2, curY, mw - 4, Theme::MenuItemHeight, Theme::MenuHighlight);
                    c.setTextColor(Theme::MenuHighlightText, Theme::MenuHighlight);
                }
                else
                {
                    c.setTextColor(Theme::TextColor, Theme::MenuBg);
                }
                c.setTextSize(1);
                int16_t th = c.fontHeight();
                int tx = mx + 6;
                int ty = curY + (Theme::MenuItemHeight - th) / 2;
                c.setCursor(tx, ty);
                c.print(items[i].label);

                // draw submenu arrow
                if (items[i].hasSubmenu())
                {
                    int arrowX = mx + mw - 12;
                    int arrowCY = curY + Theme::MenuItemHeight / 2;
                    uint16_t arrowCol = hl ? Theme::MenuHighlightText : Theme::TextColor;
                    c.fillTriangle(arrowX, arrowCY - 3,
                                   arrowX, arrowCY + 3,
                                   arrowX + 4, arrowCY,
                                   arrowCol);
                }

                curY += Theme::MenuItemHeight;
            }
        }
    };

} // namespace UI
