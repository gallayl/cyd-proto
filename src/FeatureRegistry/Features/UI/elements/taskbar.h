#pragma once

#include <functional>
#include <vector>
#include "../Renderer.h"
#include "../Theme.h"

namespace UI
{

    class Taskbar
    {
    public:
        using StartClickCb = std::function<void()>;
        using AppClickCb = std::function<void(const char *)>;

        struct TaskbarApp
        {
            String name;
            bool focused;
        };

        using KeyboardToggleCb = std::function<void()>;

        void setStartClickCallback(StartClickCb cb) { onStartClick = std::move(cb); }
        void setAppClickCallback(AppClickCb cb) { onAppClick = std::move(cb); }
        void setKeyboardToggleCallback(KeyboardToggleCb cb) { onKeyboardToggle = std::move(cb); }

        void setApps(const std::vector<TaskbarApp> &apps) { openApps = apps; }

        void setStartMenuOpen(bool open) { startMenuOpen = open; }

        void draw()
        {
            auto &c = canvas();
            int tbY = Theme::TaskbarY;
            int tbH = Theme::TaskbarHeight;

            // taskbar background
            c.fillRect(0, tbY, Theme::ScreenWidth, tbH, Theme::TaskbarBg);
            // top edge highlight
            c.drawFastHLine(0, tbY, Theme::ScreenWidth, Theme::ButtonHighlight);

            // start button
            drawButton(c, 2, tbY + 2, Theme::StartButtonWidth, tbH - 4,
                        "Start", startMenuOpen || startTouching);

            // keyboard toggle button (right side)
            int kbX = Theme::ScreenWidth - Theme::KeyboardToggleSize - 2;
            int kbY = tbY + 2;
            int kbS = Theme::KeyboardToggleSize;
            drawButton(c, kbX, kbY, kbS, tbH - 4, "Kb", kbTouching);

            // app buttons (between start and keyboard toggle)
            int appX = Theme::StartButtonWidth + 6;
            int appBtnW = 0;
            int availW = kbX - appX - 2;
            if (!openApps.empty())
            {
                appBtnW = availW / (int)openApps.size();
                if (appBtnW > 80)
                    appBtnW = 80;
            }

            for (size_t i = 0; i < openApps.size(); i++)
            {
                int bx = appX + (int)i * appBtnW;
                drawButton(c, bx, tbY + 2, appBtnW - 2, tbH - 4,
                            openApps[i].name, openApps[i].focused);
            }
        }

        bool handleTouch(int px, int py)
        {
            if (py < Theme::TaskbarY)
                return false;

            int tbY = Theme::TaskbarY;
            int tbH = Theme::TaskbarHeight;

            // start button
            if (px >= 2 && px < 2 + Theme::StartButtonWidth &&
                py >= tbY + 2 && py < tbY + tbH - 2)
            {
                startTouching = true;
                return true;
            }

            // keyboard toggle button
            int kbX = Theme::ScreenWidth - Theme::KeyboardToggleSize - 2;
            if (px >= kbX && px < kbX + Theme::KeyboardToggleSize &&
                py >= tbY + 2 && py < tbY + tbH - 2)
            {
                kbTouching = true;
                return true;
            }

            // app buttons
            int appX = Theme::StartButtonWidth + 6;
            int appBtnW = 0;
            int availW = kbX - appX - 2;
            if (!openApps.empty())
            {
                appBtnW = availW / (int)openApps.size();
                if (appBtnW > 80)
                    appBtnW = 80;
            }

            for (size_t i = 0; i < openApps.size(); i++)
            {
                int bx = appX + (int)i * appBtnW;
                if (px >= bx && px < bx + appBtnW - 2 &&
                    py >= tbY + 2 && py < tbY + tbH - 2)
                {
                    touchedAppIndex = (int)i;
                    return true;
                }
            }
            return false;
        }

        bool handleTouchEnd(int px, int py)
        {
            bool handled = false;

            // if we started the touch on the start button, fire the callback
            // regardless of where the finger is lifted.  this matches normal
            // "tap" behaviour and avoids situations where a tiny finger shift
            // causes the click to be ignored.
            if (startTouching)
            {
                if (onStartClick)
                    onStartClick();
                handled = true;
            }

            if (kbTouching)
            {
                if (onKeyboardToggle)
                    onKeyboardToggle();
                handled = true;
            }

            // now check for app buttons using the original touch-down index
            if (py >= Theme::TaskbarY)
            {
                int tbY = Theme::TaskbarY;
                int tbH = Theme::TaskbarHeight;

                int kbX = Theme::ScreenWidth - Theme::KeyboardToggleSize - 2;
                int appX = Theme::StartButtonWidth + 6;
                int appBtnW = 0;
                int availW = kbX - appX - 2;
                if (!openApps.empty())
                {
                    appBtnW = availW / (int)openApps.size();
                    if (appBtnW > 80)
                        appBtnW = 80;
                }

                if (touchedAppIndex >= 0 && touchedAppIndex < (int)openApps.size())
                {
                    int bx = appX + touchedAppIndex * appBtnW;
                    if (px >= bx && px < bx + appBtnW - 2 &&
                        py >= tbY + 2 && py < tbY + tbH - 2)
                    {
                        if (onAppClick)
                            onAppClick(openApps[touchedAppIndex].name.c_str());
                        handled = true;
                    }
                }
            }

            // reset state after handling the end of touch
            startTouching = false;
            kbTouching = false;
            touchedAppIndex = -1;
            return handled;
        }

    private:
        StartClickCb onStartClick;
        AppClickCb onAppClick;
        KeyboardToggleCb onKeyboardToggle;
        std::vector<TaskbarApp> openApps;
        bool startMenuOpen{false};
        bool startTouching{false};
        bool kbTouching{false};
        int touchedAppIndex{-1};

        static void drawButton(LGFX_Sprite &c, int bx, int by, int bw, int bh,
                                const String &text, bool pressed)
        {
            uint16_t light = pressed ? Theme::ButtonShadow : Theme::ButtonHighlight;
            uint16_t dark = pressed ? Theme::ButtonHighlight : Theme::ButtonShadow;

            c.fillRect(bx, by, bw, bh, Theme::ButtonFace);
            c.drawFastHLine(bx, by, bw, light);
            c.drawFastVLine(bx, by, bh, light);
            c.drawFastHLine(bx, by + bh - 1, bw, dark);
            c.drawFastVLine(bx + bw - 1, by, bh, dark);

            c.setTextColor(Theme::TextColor, Theme::ButtonFace);
            c.setTextSize(1);
            int16_t tw = c.textWidth(text);
            int16_t th = c.fontHeight();
            int tx = bx + (bw - tw) / 2 + (pressed ? 1 : 0);
            int ty = by + (bh - th) / 2 + (pressed ? 1 : 0);
            c.setCursor(tx, ty);
            c.print(text);
        }
    };

} // namespace UI
