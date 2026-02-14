#pragma once

#include <functional>
#include "container.h"
#include "scrollable.h"
#include "label.h"
#include "button.h"
#include "menubar.h"
#include "../Renderer.h"
#include "../Theme.h"
#include "../ActionQueue.h"

namespace UI
{

    enum class WindowState
    {
        Restored,
        Maximized,
        TopHalf,
        BottomHalf,
        Minimized
    };

    class Window : public Element
    {
    public:
        using CloseCallback = std::function<void()>;
        using MinimizeCallback = std::function<void()>;
        using StateChangeCallback = std::function<void(WindowState)>;

        Window(const String &title, int ix, int iy, int iw, int ih)
            : titleText(title),
              closeBtn("x", 0, 0, Theme::WinBtnSize, Theme::WinBtnSize),
              maxBtn("", 0, 0, Theme::WinBtnSize, Theme::WinBtnSize),
              minBtn("_", 0, 0, Theme::WinBtnSize, Theme::WinBtnSize),
              titleLabel(title, 0, 0, 0, Theme::TitleBarHeight),
              scrollableContent()
        {
            setBounds(ix, iy, iw, ih);
            scrollableContent.setBounds(contentX(), contentY(), contentW(), contentH());
            scrollableContent.getContent().setBounds(contentX(), contentY(), contentW(), contentH());
            scrollableContent.setAutoContentHeight(true);
            scrollableContent.setThinScrollbar(true);
            titleLabel.setTextSize(1);

            auto setupBtn = [](Button &btn)
            {
                btn.setTextSize(1);
                btn.setBackgroundColor(Theme::ButtonFace);
                btn.setTextColor(Theme::TextColor, Theme::ButtonFace);
                btn.setBorderColors(Theme::ButtonHighlight, Theme::ButtonShadow);
            };
            setupBtn(closeBtn);
            setupBtn(maxBtn);
            setupBtn(minBtn);

            maxBtn.setCallback([this]()
                               { cycleWindowState(); });
        }

        void setCloseCallback(CloseCallback cb)
        {
            closeBtn.setCallback(std::move(cb));
        }

        void setMinimizeCallback(MinimizeCallback cb) { onMinimize = std::move(cb); }
        void setStateChangeCallback(StateChangeCallback cb) { onStateChange = std::move(cb); }

        void setActive(bool a) { active = a; }
        bool isActive() const { return active; }

        const String &getTitle() const { return titleText; }

        Container &getContent() { return scrollableContent.getContent(); }

        // menu bar
        void setMenuBar(std::vector<MenuBarEntry> menus)
        {
            hasMenuBar = true;
            menuBar.setMenus(std::move(menus));
        }
        MenuBar *getMenuBar() { return hasMenuBar ? &menuBar : nullptr; }

        // window state
        WindowState getState() const { return winState; }
        void setState(WindowState s) { winState = s; }
        bool isMinimized() const { return winState == WindowState::Minimized; }

        void cycleWindowState()
        {
            WindowState newState;
            switch (winState)
            {
            case WindowState::Restored:
                newState = WindowState::Maximized;
                break;
            case WindowState::Maximized:
                newState = WindowState::TopHalf;
                break;
            case WindowState::TopHalf:
                newState = WindowState::BottomHalf;
                break;
            case WindowState::BottomHalf:
            default:
                newState = WindowState::Restored;
                break;
            }
            winState = newState;
            if (onStateChange)
            {
                auto cb = onStateChange;
                UI::queueAction([cb, newState]()
                                { cb(newState); });
            }
        }

        void mount() override
        {
            if (mounted)
                return;
            Element::mount();
            titleLabel.mount();
            closeBtn.mount();
            maxBtn.mount();
            minBtn.mount();
            if (hasMenuBar)
                menuBar.mount();
            scrollableContent.mount();
        }

        void unmount() override
        {
            if (!mounted)
                return;
            scrollableContent.unmount();
            if (hasMenuBar)
                menuBar.unmount();
            minBtn.unmount();
            maxBtn.unmount();
            closeBtn.unmount();
            titleLabel.unmount();
            Element::unmount();
        }

        void draw() override
        {
            if (!mounted || winState == WindowState::Minimized)
                return;
            auto &c = canvas();

            int bw = Theme::WindowBorderWidth;

            // outer border (3D raised)
            c.drawRect(x, y, width, height, Theme::WindowBorder);
            c.drawFastHLine(x + 1, y + 1, width - 2, Theme::ButtonHighlight);
            c.drawFastVLine(x + 1, y + 1, height - 2, Theme::ButtonHighlight);
            c.drawFastHLine(x + 1, y + height - 2, width - 2, Theme::ButtonShadow);
            c.drawFastVLine(x + width - 2, y + 1, height - 2, Theme::ButtonShadow);

            // title bar background
            uint16_t tbColor = active ? Theme::TitleBarActive : Theme::TitleBarInactive;
            uint16_t tbText = active ? Theme::TitleTextActive : Theme::TitleTextInactive;
            int tbX = x + bw;
            int tbY = y + bw;
            int tbW = width - bw * 2;
            int tbH = Theme::TitleBarHeight;
            c.fillRect(tbX, tbY, tbW, tbH, tbColor);

            // buttons area width (3 buttons)
            int btnSpacing = 2;
            int btnsW = Theme::WinBtnSize * 3 + btnSpacing * 2 + 2;

            // title text
            titleLabel.setBounds(tbX + 2, tbY, tbW - btnsW - 4, tbH);
            titleLabel.setTextColor(tbText, tbColor);
            titleLabel.setAlign(TextAlign::LEFT);
            titleLabel.draw();

            // title bar buttons (right side): minimize | maximize | close
            int btnY = tbY + (tbH - Theme::WinBtnSize) / 2;
            int btnX = x + width - bw - btnsW;

            minBtn.setBounds(btnX, btnY, Theme::WinBtnSize, Theme::WinBtnSize);
            minBtn.draw();
            // draw minimize icon (horizontal line at bottom)
            c.drawFastHLine(btnX + 3, btnY + Theme::WinBtnSize - 5, Theme::WinBtnSize - 6, Theme::TextColor);

            btnX += Theme::WinBtnSize + btnSpacing;
            maxBtn.setBounds(btnX, btnY, Theme::WinBtnSize, Theme::WinBtnSize);
            maxBtn.setLabel("");
            maxBtn.draw();
            // draw maximize/restore icon
            if (winState == WindowState::Restored)
            {
                // single rectangle = maximize
                c.drawRect(btnX + 3, btnY + 3, Theme::WinBtnSize - 6, Theme::WinBtnSize - 6, Theme::TextColor);
                c.drawFastHLine(btnX + 3, btnY + 4, Theme::WinBtnSize - 6, Theme::TextColor);
            }
            else
            {
                // overlapping rectangles = restore
                int sz = Theme::WinBtnSize - 8;
                c.drawRect(btnX + 5, btnY + 2, sz, sz, Theme::TextColor);
                c.drawFastHLine(btnX + 5, btnY + 3, sz, Theme::TextColor);
                c.drawRect(btnX + 3, btnY + 4, sz, sz, Theme::TextColor);
                c.drawFastHLine(btnX + 3, btnY + 5, sz, Theme::TextColor);
                c.fillRect(btnX + 3, btnY + 5, sz, sz - 1, Theme::ButtonFace);
                c.drawRect(btnX + 3, btnY + 4, sz, sz, Theme::TextColor);
                c.drawFastHLine(btnX + 3, btnY + 5, sz, Theme::TextColor);
            }

            btnX += Theme::WinBtnSize + btnSpacing;
            closeBtn.setBounds(btnX, btnY, Theme::WinBtnSize, Theme::WinBtnSize);
            closeBtn.draw();

            // menu bar
            if (hasMenuBar)
            {
                int mbY = y + bw + tbH;
                menuBar.setBounds(x + bw, mbY, width - bw * 2, Theme::MenuBarHeight);
                menuBar.draw();
            }

            // content area background
            int cX = contentX();
            int cY = contentY();
            int cW = contentW();
            int cH = contentH();
            c.fillRect(cX, cY, cW, cH, Theme::WindowBg);

            // draw scrollable content
            scrollableContent.setBounds(cX, cY, cW, cH);
            scrollableContent.draw();
        }

        void onTouch(int px, int py) override
        {
            if (!mounted || winState == WindowState::Minimized)
                return;

            // title bar buttons
            if (closeBtn.contains(px, py))
            {
                closeBtn.onTouch(px, py);
                return;
            }
            if (maxBtn.contains(px, py))
            {
                maxBtn.onTouch(px, py);
                return;
            }
            if (minBtn.contains(px, py))
            {
                minBtn.onTouch(px, py);
                return;
            }

            // menu bar
            if (hasMenuBar)
            {
                int mbY = y + Theme::WindowBorderWidth + Theme::TitleBarHeight;
                if (py >= mbY && py < mbY + Theme::MenuBarHeight)
                {
                    menuBar.onTouch(px, py);
                    return;
                }
                // if dropdown is open, check it too
                if (menuBar.isDropdownOpen())
                {
                    menuBar.onTouch(px, py);
                    return;
                }
            }

            // content area
            int cX = contentX();
            int cY = contentY();
            int cW = contentW();
            int cH = contentH();
            if (px >= cX && px < cX + cW && py >= cY && py < cY + cH)
            {
                scrollableContent.onTouch(px, py);
            }
        }

        void onTouchEnd(int px, int py) override
        {
            if (!mounted || winState == WindowState::Minimized)
                return;

            closeBtn.onTouchEnd(px, py);
            maxBtn.onTouchEnd(px, py);

            if (minBtn.contains(px, py) && minBtn.isPressed())
            {
                minBtn.onTouchEnd(px, py);
                winState = WindowState::Minimized;
                if (onMinimize)
                {
                    auto cb = onMinimize;
                    UI::queueAction([cb]()
                                    { cb(); });
                }
                return;
            }
            minBtn.onTouchEnd(px, py);

            if (hasMenuBar)
                menuBar.onTouchEnd(px, py);

            scrollableContent.onTouchEnd(px, py);
        }

        int contentX() const { return x + Theme::WindowBorderWidth; }
        int contentY() const
        {
            int cy = y + Theme::WindowBorderWidth + Theme::TitleBarHeight;
            if (hasMenuBar)
                cy += Theme::MenuBarHeight;
            return cy;
        }
        int contentW() const { return width - Theme::WindowBorderWidth * 2; }
        int contentH() const
        {
            int ch = height - Theme::WindowBorderWidth * 2 - Theme::TitleBarHeight;
            if (hasMenuBar)
                ch -= Theme::MenuBarHeight;
            return ch;
        }

    private:
        String titleText;
        bool active{true};
        WindowState winState{WindowState::Restored};

        Button closeBtn;
        Button maxBtn;
        Button minBtn;
        Label titleLabel;
        ScrollableContainer scrollableContent;

        bool hasMenuBar{false};
        MenuBar menuBar;

        MinimizeCallback onMinimize;
        StateChangeCallback onStateChange;
    };

} // namespace UI
