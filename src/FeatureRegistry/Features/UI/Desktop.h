#pragma once

#include "WindowManager.h"
#include "elements/startmenu.h"
#include "elements/keyboard.h"
#include "elements/textfield.h"
#include "elements/error_popup.h"
#include "App.h"
#include <Esp.h>
#include <map>

#include "../../../../config.h"
#if ENABLE_BERRY
#include "../Berry/BerryFeature.h"
#endif

namespace UI
{

class Desktop
{
public:
    void init()
    {
        // build Berry app name-to-path map and create launcher helper
#if ENABLE_BERRY
        auto scripts = scanBerryScripts();
        std::map<String, String> berryApps;
        for (auto &s : scripts)
            berryApps[s.name] = s.path;

        auto openBerryApp = [berryApps](const char *name)
        {
            return [berryApps, n = String(name)]()
            {
                auto it = berryApps.find(n);
                if (it != berryApps.end())
                    openBerryScript(it->second);
                else
                    errorPopup().show(("App '" + n + "' not found").c_str());
            };
        };
#else
        auto openBerryApp = [](const char *name)
        {
            return [n = String(name)]()
            {
                errorPopup().show("Berry is not enabled");
            };
        };
#endif

        // build hierarchical menu
        std::vector<MenuItem> menuItems = {
            MenuItem::Submenu("Programs",
                              {
                                  MenuItem::Leaf("RGB LED", openBerryApp("RGB LED")),
                                  MenuItem::Leaf("Sensors", openBerryApp("Sensors")),
                                  MenuItem::Leaf("Paint", openBerryApp("Paint")),
                              }),
            MenuItem::Submenu("Settings",
                              {
                                  MenuItem::Leaf("WiFi", openBerryApp("WiFi")),
                                  MenuItem::Leaf("Display", openBerryApp("Display")),
                              }),
            MenuItem::Submenu("System",
                              {
                                  MenuItem::Leaf("Info", openBerryApp("Info")),
                                  MenuItem::Leaf("Features", openBerryApp("Features")),
                                  MenuItem::Leaf("Log Viewer", openBerryApp("Log Viewer")),
                                  MenuItem::Leaf("File Manager", openBerryApp("File Manager")),
                              }),
        };

        menuItems.push_back(MenuItem::Separator());
        menuItems.push_back(MenuItem::Leaf("Restart", []() { ESP.restart(); }));

        startMenu.setMenuItems(std::move(menuItems));

        // keyboard focus routing for TextFields
        setKeyboardFocusHandler(
            [this](std::function<void(char)> consumer)
            {
                keyConsumer = std::move(consumer);
                keyboard.setOnKeyPress(
                    [this](char ch)
                    {
                        if (keyConsumer)
                            keyConsumer(ch);
                        markDirty();
                    });
                if (!keyboard.isVisible())
                {
                    keyboard.show();
                    windowManager().setKeyboardVisible(true);
                }
            });

        setKeyboardBlurHandler(
            [this]()
            {
                keyConsumer = nullptr;
                keyboard.setOnKeyPress(nullptr);
                if (keyboard.isVisible())
                {
                    keyboard.hide();
                    windowManager().setKeyboardVisible(false);
                }
            });

        // wire overlay callbacks into window manager
        windowManager().setOverlayDraw([this]() { drawOverlays(); });

        windowManager().setOverlayTouch([this](int px, int py) -> bool { return handleOverlayTouch(px, py); });

        windowManager().setOverlayTouchEnd([this](int px, int py) -> bool { return handleOverlayTouchEnd(px, py); });
    }

    void draw()
    {
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

    void toggleStartMenu()
    {
        startMenu.toggle();
    }

    void toggleKeyboard()
    {
        keyboard.toggle();
        windowManager().setKeyboardVisible(keyboard.isVisible());
    }

private:
    StartMenu startMenu;
    Keyboard keyboard;
    std::function<void(char)> keyConsumer;

    void drawOverlays()
    {
        keyboard.draw();
        auto *panel = windowManager().getPanelSlot();
        if (panel)
        {
            auto &c = canvas();
            c.fillRect(0, Theme::TaskbarY, Theme::ScreenWidth, Theme::TaskbarHeight, Theme::TaskbarBg);
            c.drawFastHLine(0, Theme::TaskbarY, Theme::ScreenWidth, Theme::ButtonHighlight);
            panel->container->draw();
        }
        startMenu.draw();
        errorPopup().draw();
    }

    bool handleOverlayTouch(int px, int py)
    {
        // error popup gets highest priority
        if (errorPopup().isVisible())
        {
            if (errorPopup().handleTouch(px, py))
                return true;
        }
        // start menu gets priority if visible
        if (startMenu.isVisible())
        {
            if (startMenu.handleTouch(px, py))
                return true;
        }
        if (keyboard.handleTouch(px, py))
            return true;
        // panel (taskbar) touch
        auto *panel = windowManager().getPanelSlot();
        if (panel && py >= Theme::TaskbarY)
        {
            panel->container->handleTouch(px, py);
            return true;
        }
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
        if (errorPopup().isVisible())
        {
            if (errorPopup().handleTouchEnd(px, py))
                return true;
        }
        if (startMenu.isVisible())
        {
            if (startMenu.handleTouchEnd(px, py))
                return true;
        }
        if (keyboard.handleTouchEnd(px, py))
            return true;
        auto *panel = windowManager().getPanelSlot();
        if (panel && py >= Theme::TaskbarY)
        {
            panel->container->handleTouchEnd(px, py);
            return true;
        }
        return false;
    }
};

inline Desktop &desktop()
{
    static Desktop d;
    return d;
}

} // namespace UI
