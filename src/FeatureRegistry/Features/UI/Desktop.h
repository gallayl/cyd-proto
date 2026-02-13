#pragma once

#include "WindowManager.h"
#include "elements/taskbar.h"
#include "elements/startmenu.h"
#include "App.h"
#include "apps/RgbLedApp.h"
#include "apps/InfoApp.h"
#include "apps/WifiApp.h"

namespace UI
{

    class Desktop
    {
    public:
        void init()
        {
            registerApp("RGB LED", []() -> App *
                        { return new RgbLedApp(); });
            registerApp("Info", []() -> App *
                        { return new InfoApp(); });
            registerApp("WiFi", []() -> App *
                        { return new WifiApp(); });

            taskbar.setStartClickCallback([this]()
                                          { startMenu.toggle(); });

            taskbar.setAppClickCallback([](const char *name)
                                        { windowManager().focusApp(name); });

            startMenu.setAppSelectedCallback([this](const char *name)
                                             { windowManager().openApp(name); });

            // wire overlay callbacks into window manager
            windowManager().setOverlayDraw([this]()
                                           { drawOverlays(); });

            windowManager().setOverlayTouch([this](int px, int py) -> bool
                                            { return handleOverlayTouch(px, py); });

            windowManager().setOverlayTouchEnd([this](int px, int py) -> bool
                                               { return handleOverlayTouchEnd(px, py); });
        }

        void draw()
        {
            updateTaskbarApps();
            windowManager().draw();
            flush();
        }

        void handleTouch(int px, int py)
        {
            windowManager().handleTouch(px, py);
        }

        void handleTouchEnd(int px, int py)
        {
            windowManager().handleTouchEnd(px, py);
        }

        void loop()
        {
            windowManager().loopApps();
        }

    private:
        Taskbar taskbar;
        StartMenu startMenu;

        void updateTaskbarApps()
        {
            std::vector<Taskbar::TaskbarApp> apps;
            auto *focused = windowManager().getFocused();
            for (auto &oa : windowManager().getOpenApps())
            {
                bool isFocused = (focused && &oa == focused);
                apps.push_back({oa.name, isFocused});
            }
            taskbar.setApps(apps);
            taskbar.setStartMenuOpen(startMenu.isVisible());
        }

        void drawOverlays()
        {
            taskbar.draw();
            startMenu.draw();
        }

        bool handleOverlayTouch(int px, int py)
        {
            // start menu gets priority if visible
            if (startMenu.isVisible())
            {
                if (startMenu.handleTouch(px, py))
                    return true;
            }
            if (taskbar.handleTouch(px, py))
                return true;
            // touch outside start menu closes it
            if (startMenu.isVisible())
            {
                startMenu.hide();
                return true;
            }
            return false;
        }

        bool handleOverlayTouchEnd(int px, int py)
        {
            if (startMenu.isVisible())
            {
                if (startMenu.handleTouchEnd(px, py))
                    return true;
            }
            if (taskbar.handleTouchEnd(px, py))
                return true;
            return false;
        }
    };

    inline Desktop &desktop()
    {
        static Desktop d;
        return d;
    }

} // namespace UI
