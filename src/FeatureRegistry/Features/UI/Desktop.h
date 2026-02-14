#pragma once

#include "WindowManager.h"
#include "elements/taskbar.h"
#include "elements/startmenu.h"
#include "elements/keyboard.h"
#include "App.h"
#include "apps/RgbLedApp.h"
#include "apps/InfoApp.h"
#include "apps/WifiApp.h"
#include "apps/SensorsApp.h"
#include "apps/LogViewerApp.h"
#include "apps/FileManagerApp.h"
#include "apps/DisplaySettingsApp.h"
#include "apps/FeaturesApp.h"
#include "apps/PaintApp.h"
#include <ESP.h>

namespace UI
{

    class Desktop
    {
    public:
        void init()
        {
            // register all apps
            registerApp("RGB LED", []() -> App *
                        { return new RgbLedApp(); });
            registerApp("Sensors", []() -> App *
                        { return new SensorsApp(); });
            registerApp("Info", []() -> App *
                        { return new InfoApp(); });
            registerApp("WiFi", []() -> App *
                        { return new WifiApp(); });
            registerApp("Display", []() -> App *
                        { return new DisplaySettingsApp(); });
            registerApp("Features", []() -> App *
                        { return new FeaturesApp(); });
            registerApp("Log Viewer", []() -> App *
                        { return new LogViewerApp(); });
            registerApp("File Manager", []() -> App *
                        { return new FileManagerApp(); });
            registerApp("Paint", []() -> App *
                        { return new PaintApp(); });

            // build hierarchical menu
            auto openApp = [this](const char *name)
            {
                return [this, name]()
                {
                    if (onAppSelected)
                        onAppSelected(name);
                };
            };

            startMenu.setMenuItems({
                MenuItem::Submenu("Programs", {
                                                  MenuItem::Leaf("RGB LED", openApp("RGB LED")),
                                                  MenuItem::Leaf("Sensors", openApp("Sensors")),
                                                  MenuItem::Leaf("Paint", openApp("Paint")),
                                              }),
                MenuItem::Submenu("Settings", {
                                                  MenuItem::Leaf("WiFi", openApp("WiFi")),
                                                  MenuItem::Leaf("Display", openApp("Display")),
                                              }),
                MenuItem::Submenu("System", {
                                                MenuItem::Leaf("Info", openApp("Info")),
                                                MenuItem::Leaf("Features", openApp("Features")),
                                                MenuItem::Leaf("Log Viewer", openApp("Log Viewer")),
                                                MenuItem::Leaf("File Manager", openApp("File Manager")),
                                            }),
                MenuItem::Separator(),
                MenuItem::Leaf("Restart", []()
                               { ESP.restart(); }),
            });

            taskbar.setStartClickCallback([this]()
                                          { startMenu.toggle(); });

            taskbar.setAppClickCallback([](const char *name)
                                        { windowManager().focusApp(name); });

            taskbar.setKeyboardToggleCallback([this]()
                                              {
                                                  keyboard.toggle();
                                                  windowManager().setKeyboardVisible(keyboard.isVisible());
                                              });

            onAppSelected = [](const char *name)
            { windowManager().openApp(name); };

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

        void tickTimers()
        {
            windowManager().tickTimers();
        }

    private:
        Taskbar taskbar;
        StartMenu startMenu;
        Keyboard keyboard;
        std::function<void(const char *)> onAppSelected;

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
            keyboard.draw();
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
            if (keyboard.handleTouch(px, py))
                return true;
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
            if (keyboard.handleTouchEnd(px, py))
                return true;
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
