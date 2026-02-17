#include "../../../config.h"

#if ENABLE_UI

#include "WindowManager.h"
#include "esp_log.h"

namespace UI
{

WindowManager &windowManager()
{
    static WindowManager wm;
    return wm;
}

static void setupWindowCallbacks(WindowManager &wm, OpenApp &oa)
{
    WindowManager *wmPtr = &wm;
    oa.window->setCloseCallback([wmPtr, name = oa.name]() { wmPtr->closeApp(name.c_str()); });
    oa.window->setMinimizeCallback([wmPtr, name = oa.name]() { wmPtr->minimizeApp(name.c_str()); });
    oa.window->setStateChangeCallback([wmPtr, name = oa.name](WindowState state)
                                      { wmPtr->handleWindowStateChange(name.c_str(), state); });

    if (oa.app && oa.app->hasIcon())
    {
        App *appPtr = oa.app.get();
        oa.window->setIconDrawer([appPtr](LGFX_Sprite &c, int x, int y, int sz) { appPtr->drawIcon(c, x, y, sz); });
    }
}

void WindowManager::openApp(const char *appName)
{
    // if already open, just focus (restore if minimized)
    for (auto &oa : _openApps)
    {
        if (oa.name == appName)
        {
            if (oa.window->isMinimized())
            {
                restoreApp(appName);
            }
            else
            {
                focusApp(appName);
            }
            return;
        }
    }

    // find factory in registry
    for (auto &entry : appRegistry())
    {
        if (std::string(entry.name) == appName)
        {
            openApp(appName, entry.factory());
            return;
        }
    }
}

void WindowManager::openApp(const char *appName, App *appInstance)
{
    if (appInstance == nullptr)
    {
        return;
    }

    // if already open, just focus (restore if minimized)
    for (auto &oa : _openApps)
    {
        if (oa.name == appName)
        {
            if (oa.window->isMinimized())
            {
                restoreApp(appName);
            }
            else
            {
                focusApp(appName);
            }
            delete appInstance;
            return;
        }
    }

    OpenApp oa;
    oa.name = appName;
    oa.app.reset(appInstance);

    int winX = Theme::WindowBorderWidth;
    int winY = Theme::DesktopY + 2;
    int winW = Theme::ScreenWidth() - (Theme::WindowBorderWidth * 2);
    int winH = availableDesktopHeight() - 4;

    oa.window = std::make_unique<Window>(appName, winX, winY, winW, winH);

    setupWindowCallbacks(*this, oa);

    oa.window->mount();
    oa.app->setup(oa.window->getContent(), oa.window->contentW(), oa.window->contentH());

    _openApps.push_back(std::move(oa));
    updateActiveStates();
    markDirty();
}

void WindowManager::closeApp(const char *appName)
{
    auto it = std::find_if(_openApps.begin(), _openApps.end(), [&](const OpenApp &oa) { return oa.name == appName; });
    if (it != _openApps.end())
    {
        it->app->teardown();
        it->window->unmount();
        _openApps.erase(it);
        updateActiveStates();
        markDirty();
    }
}

void WindowManager::focusApp(const char *appName)
{
    auto it = std::find_if(_openApps.begin(), _openApps.end(), [&](const OpenApp &oa) { return oa.name == appName; });
    if (it != _openApps.end() && it != _openApps.end() - 1)
    {
        OpenApp tmp = std::move(*it);
        _openApps.erase(it);
        _openApps.push_back(std::move(tmp));
        markDirty();
    }
    updateActiveStates();
}

void WindowManager::minimizeApp(const char *appName)
{
    auto it = std::find_if(_openApps.begin(), _openApps.end(), [&](const OpenApp &oa) { return oa.name == appName; });
    if (it != _openApps.end())
    {
        it->window->setState(WindowState::Minimized);
        markDirty();
    }
}

void WindowManager::restoreApp(const char *appName)
{
    auto it = std::find_if(_openApps.begin(), _openApps.end(), [&](const OpenApp &oa) { return oa.name == appName; });
    if (it != _openApps.end())
    {
        if (it->window->isMinimized())
        {
            it->window->setState(WindowState::Restored);
        }
        focusApp(appName);
        markDirty();
    }
}

void WindowManager::handleWindowStateChange(const char *appName, WindowState state)
{
    auto it = std::find_if(_openApps.begin(), _openApps.end(), [&](const OpenApp &oa) { return oa.name == appName; });
    if (it == _openApps.end())
    {
        return;
    }

    int winX = Theme::WindowBorderWidth;
    int winY = Theme::DesktopY + 2;
    int winW = Theme::ScreenWidth() - (Theme::WindowBorderWidth * 2);
    int deskH = availableDesktopHeight();
    int winH = deskH - 4;

    switch (state)
    {
    case WindowState::Maximized:
        winX = 0;
        winY = Theme::DesktopY;
        winW = Theme::ScreenWidth();
        winH = deskH;
        break;
    case WindowState::TopHalf:
        winX = 0;
        winY = Theme::DesktopY;
        winW = Theme::ScreenWidth();
        winH = deskH / 2;
        break;
    case WindowState::BottomHalf:
        winX = 0;
        winY = Theme::DesktopY + (deskH / 2);
        winW = Theme::ScreenWidth();
        winH = deskH / 2;
        break;
    case WindowState::Minimized:
        // handled by minimizeApp
        return;
    case WindowState::Restored:
    default:
        break;
    }

    // rebuild window with new bounds
    it->app->teardown();
    it->window->unmount();

    it->window = std::make_unique<Window>(it->name.c_str(), winX, winY, winW, winH);
    it->window->setState(state);

    setupWindowCallbacks(*this, *it);

    it->window->mount();
    it->app->setup(it->window->getContent(), it->window->contentW(), it->window->contentH());

    updateActiveStates();
    markDirty();
}

bool WindowManager::isAppOpen(const char *appName) const
{
    for (const auto &oa : _openApps)
    {
        if (oa.name == appName)
        {
            return true;
        }
    }
    return false;
}

OpenApp *WindowManager::getFocused()
{
    if (_openApps.empty())
    {
        return nullptr;
    }
    // find topmost non-minimized
    for (int i = (int)_openApps.size() - 1; i >= 0; i--)
    {
        if (!_openApps[i].window->isMinimized())
        {
            return &_openApps[i];
        }
    }
    return nullptr;
}

void WindowManager::updateActiveStates()
{
    auto *focused = getFocused();
    for (auto &oa : _openApps)
    {
        oa.window->setActive((focused != nullptr) && &oa == focused);
    }
}

void WindowManager::draw()
{
    auto &c = canvas();
    // desktop background
    c.fillRect(0, Theme::DesktopY - stripOffsetY(), Theme::ScreenWidth(), Theme::DesktopHeight(), Theme::DesktopBg);

    // draw windows in z-order (back to front), skip minimized
    for (auto &oa : _openApps)
    {
        if (!oa.window->isMinimized())
        {
            oa.window->draw();
        }
    }

    // overlay (taskbar, start menu)
    if (_overlayDraw)
    {
        _overlayDraw();
    }
}

void WindowManager::handleTouch(int px, int py)
{
    // overlay gets first crack (taskbar, start menu)
    if (_overlayTouch && _overlayTouch(px, py))
    {
        return;
    }

    // hit-test windows top-down (back of vector = top), skip minimized
    for (int i = (int)_openApps.size() - 1; i >= 0; i--)
    {
        if (_openApps[i].window->isMinimized())
        {
            continue;
        }
        if (_openApps[i].window->contains(px, py))
        {
            // focus this window if not already focused
            auto *focused = getFocused();
            if (&_openApps[i] != focused)
            {
                std::string name = _openApps[i].name;
                focusApp(name.c_str());
            }
            // top non-minimized window gets the touch
            auto *nowFocused = getFocused();
            if (nowFocused != nullptr)
            {
                nowFocused->window->onTouch(px, py);
            }
            return;
        }
    }
}

void WindowManager::handleTouchEnd(int px, int py)
{
    if (_overlayTouchEnd && _overlayTouchEnd(px, py))
    {
        return;
    }

    // forward to focused (non-minimized) window
    auto *focused = getFocused();
    if (focused != nullptr)
    {
        focused->window->onTouchEnd(px, py);
    }
}

void WindowManager::openPanel(const char *name, App *appInstance, int x, int y, int w, int h)
{
    if (appInstance == nullptr)
    {
        return;
    }

    auto slot = std::make_unique<PanelSlot>();
    slot->name = name;
    slot->app.reset(appInstance);
    slot->container = std::make_unique<Container>();
    slot->container->setBounds(x, y, w, h);
    slot->container->mount();
    slot->app->setup(*slot->container, w, h);

    _panelSlot = std::move(slot);
    markDirty();
}

void WindowManager::closePanel(const char *name)
{
    if (_panelSlot && _panelSlot->name == name)
    {
        _panelSlot->app->teardown();
        _panelSlot->container->unmount();
        _panelSlot.reset();
        markDirty();
    }
}

void WindowManager::tickTimers()
{
    for (auto &oa : _openApps)
    {
        if (oa.app->tickTimers())
        {
            markDirty();
        }
    }
    if (_panelSlot && _panelSlot->app->tickTimers())
    {
        markDirty();
    }
}

int WindowManager::availableDesktopHeight() const
{
    if (_keyboardVisible)
    {
        return Theme::DesktopHeight() - Theme::KeyboardHeight();
    }
    return Theme::DesktopHeight();
}

void WindowManager::setKeyboardVisible(bool vis)
{
    if (vis == _keyboardVisible)
    {
        return;
    }
    _keyboardVisible = vis;
    relayoutWindows();
    markDirty();
}

void WindowManager::relayoutAll()
{
    relayoutWindows();

    // Destroy all popups (they'll be recreated by panel setup)
    _popupSlots.clear();

    if (_panelSlot)
    {
        _panelSlot->app->teardown();
        _panelSlot->container->unmount();
        _panelSlot->container->clear(); // Remove old children before re-setup
        _panelSlot->container->setBounds(0, Theme::TaskbarY(), Theme::ScreenWidth(), Theme::TaskbarHeight);
        _panelSlot->container->mount();
        _panelSlot->app->setup(*_panelSlot->container, Theme::ScreenWidth(), Theme::TaskbarHeight);
    }
    markDirty();
}

void WindowManager::relayoutWindows()
{
    int winX = Theme::WindowBorderWidth;
    int winY = Theme::DesktopY + 2;
    int winW = Theme::ScreenWidth() - (Theme::WindowBorderWidth * 2);
    int winH = availableDesktopHeight() - 4;

    for (auto &oa : _openApps)
    {
        oa.app->teardown();
        oa.window->unmount();

        WindowState prevState = oa.window->getState();

        oa.window = std::make_unique<Window>(oa.name.c_str(), winX, winY, winW, winH);
        oa.window->setState(prevState);

        setupWindowCallbacks(*this, oa);

        oa.window->mount();
        oa.app->setup(oa.window->getContent(), oa.window->contentW(), oa.window->contentH());
    }
    updateActiveStates();
}

// --- Popup overlay methods ---

PopupContainer *WindowManager::createPopup(int x, int y, int w, int h, void *owner)
{
    auto p = std::make_unique<PopupContainer>(x, y, w, h);
    PopupContainer *raw = p.get();
    _popupSlots.push_back({std::move(p), owner});
    return raw;
}

void WindowManager::destroyPopup(PopupContainer *p)
{
    if (p == nullptr)
    {
        return;
    }
    auto it =
        std::find_if(_popupSlots.begin(), _popupSlots.end(), [p](const PopupSlot &s) { return s.popup.get() == p; });
    if (it != _popupSlots.end())
    {
        _popupSlots.erase(it);
        markDirty();
    }
}

void WindowManager::destroyPopupsForOwner(void *owner)
{
    _popupSlots.erase(std::remove_if(_popupSlots.begin(), _popupSlots.end(),
                                     [owner](const PopupSlot &s) { return s.owner == owner; }),
                      _popupSlots.end());
    markDirty();
}

bool WindowManager::hasVisiblePopups() const
{
    for (const auto &s : _popupSlots)
    {
        if (s.popup->isVisible())
        {
            return true;
        }
    }
    return false;
}

void WindowManager::hideAllPopups()
{
    for (auto &s : _popupSlots)
    {
        s.popup->hide();
    }
    markDirty();
}

void WindowManager::drawPopups()
{
    static int logCount = 0;
    for (auto &s : _popupSlots)
    {
        if (s.popup->isVisible())
        {
            if (logCount < 5)
            {
                int bx;
                int by;
                int bw;
                int bh;
                s.popup->getBounds(bx, by, bw, bh);
                ESP_LOGI("WindowManager", "drawPopups: visible popup at (%d,%d,%d,%d) mounted=%d children=%d", bx, by,
                         bw, bh, static_cast<int>(s.popup->isMounted()), (int)s.popup->getChildren().size());
                logCount++;
            }
            s.popup->draw();
        }
    }
}

bool WindowManager::handlePopupTouch(int px, int py)
{
    bool anyVisible = hasVisiblePopups();
    if (!anyVisible)
    {
        return false;
    }

    // check if touch is inside any visible popup (iterate in reverse for z-order)
    for (int i = (int)_popupSlots.size() - 1; i >= 0; i--)
    {
        if (_popupSlots[i].popup->isVisible() && _popupSlots[i].popup->contains(px, py))
        {
            _popupSlots[i].popup->handleTouch(px, py);
            return true;
        }
    }

    // touch is outside all visible popups — dismiss them
    hideAllPopups();
    return true;
}

bool WindowManager::handlePopupTouchEnd(int px, int py)
{
    bool anyVisible = hasVisiblePopups();
    if (!anyVisible)
    {
        return false;
    }

    for (int i = (int)_popupSlots.size() - 1; i >= 0; i--)
    {
        if (_popupSlots[i].popup->isVisible() && _popupSlots[i].popup->contains(px, py))
        {
            _popupSlots[i].popup->handleTouchEnd(px, py);
            return true;
        }
    }

    hideAllPopups();
    return true;
}

} // namespace UI

#endif
