#pragma once

#include <functional>
#include <vector>
#include <memory>
#include "container.h"
#include "../Renderer.h"
#include "../Theme.h"

namespace UI
{

    class TabControl : public Element
    {
    public:
        using ChangeCb = std::function<void(int)>;

        TabControl(int ix = 0, int iy = 0, int iw = 0, int ih = 0)
        {
            setBounds(ix, iy, iw, ih);
        }

        int addTab(const String &label)
        {
            Tab tab;
            tab.label = label;
            tab.content = std::make_unique<Container>();
            updateTabContentBounds(*tab.content);
            if (mounted)
                tab.content->mount();
            tabs.push_back(std::move(tab));
            return (int)tabs.size() - 1;
        }

        Container *getTabContent(int idx)
        {
            if (idx >= 0 && idx < (int)tabs.size())
                return tabs[idx].content.get();
            return nullptr;
        }

        void setActiveTab(int idx)
        {
            if (idx >= 0 && idx < (int)tabs.size())
                activeTab = idx;
        }
        int getActiveTab() const { return activeTab; }

        void setOnChange(ChangeCb cb) { onChange = std::move(cb); }

        void setBounds(int ix, int iy, int iw, int ih)
        {
            Element::setBounds(ix, iy, iw, ih);
            for (auto &tab : tabs)
                updateTabContentBounds(*tab.content);
        }

        void mount() override
        {
            if (mounted)
                return;
            Element::mount();
            for (auto &tab : tabs)
                tab.content->mount();
        }

        void unmount() override
        {
            if (!mounted)
                return;
            for (auto &tab : tabs)
                tab.content->unmount();
            Element::unmount();
        }

        void draw() override
        {
            if (!mounted)
                return;
            auto &c = canvas();

            int tabCount = (int)tabs.size();
            if (tabCount == 0)
                return;

            int tabBarY = y;
            int tabH = Theme::TabBarHeight;

            // content area background (below tabs)
            int cY = tabBarY + tabH;
            int cH = height - tabH;
            c.fillRect(x, cY, width, cH, Theme::WindowBg);

            // content area 3D raised border
            c.drawFastHLine(x, cY, width, Theme::ButtonHighlight);
            c.drawFastVLine(x, cY, cH, Theme::ButtonHighlight);
            c.drawFastHLine(x, cY + cH - 1, width, Theme::ButtonShadow);
            c.drawFastVLine(x + width - 1, cY, cH, Theme::ButtonShadow);

            // draw tab buttons
            int tabW = width / tabCount;
            c.setTextSize(1);

            for (int i = 0; i < tabCount; i++)
            {
                int tx = x + i * tabW;
                int tw = (i < tabCount - 1) ? tabW : (width - i * tabW);
                bool active = (i == activeTab);

                if (active)
                {
                    // active tab: taller, no bottom border, merges with content
                    c.fillRect(tx, tabBarY, tw, tabH + 1, Theme::WindowBg);
                    c.drawFastHLine(tx, tabBarY, tw, Theme::ButtonHighlight);
                    c.drawFastVLine(tx, tabBarY, tabH + 1, Theme::ButtonHighlight);
                    c.drawFastVLine(tx + tw - 1, tabBarY, tabH + 1, Theme::ButtonShadow);
                }
                else
                {
                    // inactive tab: shorter, with bottom border
                    int inactiveY = tabBarY + 2;
                    c.fillRect(tx + 1, inactiveY, tw - 2, tabH - 2, Theme::ButtonFace);
                    c.drawFastHLine(tx + 1, inactiveY, tw - 2, Theme::ButtonHighlight);
                    c.drawFastVLine(tx, inactiveY, tabH - 2, Theme::ButtonHighlight);
                    c.drawFastVLine(tx + tw - 1, inactiveY, tabH - 2, Theme::ButtonShadow);
                }

                // tab label
                int16_t textW = c.textWidth(tabs[i].label);
                int16_t th = c.fontHeight();
                int labelY = active ? tabBarY : (tabBarY + 2);
                int labelH = active ? tabH : (tabH - 2);
                uint16_t bg = active ? Theme::WindowBg : Theme::ButtonFace;
                c.setTextColor(Theme::TextColor, bg);
                c.setCursor(tx + (tw - textW) / 2, labelY + (labelH - th) / 2);
                c.print(tabs[i].label);
            }

            // draw active tab content
            if (activeTab >= 0 && activeTab < tabCount)
            {
                tabs[activeTab].content->draw();
            }
        }

        void onTouch(int px, int py) override
        {
            if (!mounted)
                return;

            int tabBarY = y;
            int tabH = Theme::TabBarHeight;

            // check tab bar
            if (py >= tabBarY && py < tabBarY + tabH)
            {
                int tabCount = (int)tabs.size();
                if (tabCount == 0)
                    return;
                int tabW = width / tabCount;
                int idx = (px - x) / tabW;
                if (idx >= 0 && idx < tabCount)
                    touchedTab = idx;
                return;
            }

            // forward to active tab content
            if (activeTab >= 0 && activeTab < (int)tabs.size())
            {
                tabs[activeTab].content->handleTouch(px, py);
            }
        }

        void onTouchEnd(int px, int py) override
        {
            if (!mounted)
                return;

            if (touchedTab >= 0)
            {
                int tabCount = (int)tabs.size();
                int tabW = width / tabCount;
                int idx = (px - x) / tabW;
                if (idx == touchedTab && idx != activeTab)
                {
                    activeTab = idx;
                    if (onChange)
                        onChange(activeTab);
                }
                touchedTab = -1;
                return;
            }

            if (activeTab >= 0 && activeTab < (int)tabs.size())
            {
                tabs[activeTab].content->handleTouchEnd(px, py);
            }
        }

    private:
        struct Tab
        {
            String label;
            std::unique_ptr<Container> content;
        };

        std::vector<Tab> tabs;
        int activeTab{0};
        int touchedTab{-1};
        ChangeCb onChange;

        void updateTabContentBounds(Container &c)
        {
            int cX = x + 2;
            int cY = y + Theme::TabBarHeight + 2;
            int cW = width - 4;
            int cH = height - Theme::TabBarHeight - 4;
            c.setBounds(cX, cY, cW, cH);
        }
    };

} // namespace UI
