#pragma once

#include <vector>
#include <memory>
#include <algorithm>
#include "elements/window.h"
#include "App.h"
#include "Renderer.h"
#include "Theme.h"

namespace UI
{
    struct OpenApp
    {
        std::unique_ptr<Window> window;
        std::unique_ptr<App> app;
        String name;
    };

    class WindowManager
    {
    public:
        WindowManager() = default;

        void openApp(const char *appName);
        void openApp(const char *appName, App *appInstance);
        void closeApp(const char *appName);
        void focusApp(const char *appName);
        void minimizeApp(const char *appName);
        void restoreApp(const char *appName);
        void handleWindowStateChange(const char *appName, WindowState state);
        bool isAppOpen(const char *appName) const;
        OpenApp *getFocused();
        std::vector<OpenApp> &getOpenApps() { return openApps; }
        const std::vector<OpenApp> &getOpenApps() const { return openApps; }

        void draw();
        void handleTouch(int px, int py);
        void handleTouchEnd(int px, int py);

        // taskbar/start menu need to draw on top, so they call back
        using OverlayDrawFn = std::function<void()>;
        void setOverlayDraw(OverlayDrawFn fn) { overlayDraw = std::move(fn); }

        using OverlayTouchFn = std::function<bool(int, int)>;
        void setOverlayTouch(OverlayTouchFn fn) { overlayTouch = std::move(fn); }

        using OverlayTouchEndFn = std::function<bool(int, int)>;
        void setOverlayTouchEnd(OverlayTouchEndFn fn) { overlayTouchEnd = std::move(fn); }

        void tickTimers();

        void setKeyboardVisible(bool vis);
        bool isKeyboardVisible() const { return keyboardVisible; }
        int availableDesktopHeight() const;

    private:
        std::vector<OpenApp> openApps;
        OverlayDrawFn overlayDraw;
        OverlayTouchFn overlayTouch;
        OverlayTouchEndFn overlayTouchEnd;
        bool keyboardVisible{false};

        void updateActiveStates();
        void relayoutWindows();
    };

    WindowManager &windowManager();

} // namespace UI
