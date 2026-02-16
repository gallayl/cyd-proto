#pragma once

#include <vector>
#include <memory>
#include "elements/window.h"
#include "elements/popup.h"
#include "App.h"

namespace UI
{
struct OpenApp
{
    std::unique_ptr<Window> window;
    std::unique_ptr<App> app;
    String name;
};

struct PanelSlot
{
    std::unique_ptr<Container> container;
    std::unique_ptr<App> app;
    String name;
};

struct PopupSlot
{
    std::unique_ptr<PopupContainer> popup;
    void *owner;
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
    std::vector<OpenApp> &getOpenApps()
    {
        return _openApps;
    }
    const std::vector<OpenApp> &getOpenApps() const
    {
        return _openApps;
    }

    void draw();
    void handleTouch(int px, int py);
    void handleTouchEnd(int px, int py);

    // taskbar/start menu need to draw on top, so they call back
    using OverlayDrawFn = std::function<void()>;
    void setOverlayDraw(OverlayDrawFn fn)
    {
        _overlayDraw = std::move(fn);
    }

    using OverlayTouchFn = std::function<bool(int, int)>;
    void setOverlayTouch(OverlayTouchFn fn)
    {
        _overlayTouch = std::move(fn);
    }

    using OverlayTouchEndFn = std::function<bool(int, int)>;
    void setOverlayTouchEnd(OverlayTouchEndFn fn)
    {
        _overlayTouchEnd = std::move(fn);
    }

    void tickTimers();

    void setKeyboardVisible(bool vis);
    bool isKeyboardVisible() const
    {
        return _keyboardVisible;
    }
    int availableDesktopHeight() const;

    void relayoutAll();

    // panel (frameless, always-visible app slot)
    void openPanel(const char *name, App *appInstance, int x, int y, int w, int h);
    void closePanel(const char *name);
    PanelSlot *getPanelSlot()
    {
        return _panelSlot ? _panelSlot.get() : nullptr;
    }

    // popup overlays
    PopupContainer *createPopup(int x, int y, int w, int h, void *owner);
    void destroyPopup(PopupContainer *p);
    void destroyPopupsForOwner(void *owner);
    bool hasVisiblePopups() const;
    void hideAllPopups();
    void drawPopups();
    bool handlePopupTouch(int px, int py);
    bool handlePopupTouchEnd(int px, int py);

private:
    std::vector<OpenApp> _openApps;
    std::unique_ptr<PanelSlot> _panelSlot;
    std::vector<PopupSlot> _popupSlots;
    OverlayDrawFn _overlayDraw;
    OverlayTouchFn _overlayTouch;
    OverlayTouchEndFn _overlayTouchEnd;
    bool _keyboardVisible{false};

    void updateActiveStates();
    void relayoutWindows();
};

WindowManager &windowManager();

} // namespace UI
