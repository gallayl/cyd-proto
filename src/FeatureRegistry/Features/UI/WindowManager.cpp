#include "../../../config.h"

#if ENABLE_UI

#include "WindowManager.h"

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
        oa.window->setCloseCallback([wmPtr, name = String(oa.name)]()
                                    { wmPtr->closeApp(name.c_str()); });
        oa.window->setMinimizeCallback([wmPtr, name = String(oa.name)]()
                                       { wmPtr->minimizeApp(name.c_str()); });
        oa.window->setStateChangeCallback([wmPtr, name = String(oa.name)](WindowState state)
                                          { wmPtr->handleWindowStateChange(name.c_str(), state); });
    }

    void WindowManager::openApp(const char *appName)
    {
        // if already open, just focus (restore if minimized)
        for (auto &oa : openApps)
        {
            if (oa.name == appName)
            {
                if (oa.window->isMinimized())
                    restoreApp(appName);
                else
                    focusApp(appName);
                return;
            }
        }

        // find factory in registry
        for (auto &entry : appRegistry())
        {
            if (String(entry.name) == appName)
            {
                openApp(appName, entry.factory());
                return;
            }
        }
    }

    void WindowManager::openApp(const char *appName, App *appInstance)
    {
        if (!appInstance)
            return;

        // if already open, just focus (restore if minimized)
        for (auto &oa : openApps)
        {
            if (oa.name == appName)
            {
                if (oa.window->isMinimized())
                    restoreApp(appName);
                else
                    focusApp(appName);
                delete appInstance;
                return;
            }
        }

        OpenApp oa;
        oa.name = appName;
        oa.app.reset(appInstance);

        int winX = Theme::WindowBorderWidth;
        int winY = Theme::DesktopY + 2;
        int winW = Theme::ScreenWidth - Theme::WindowBorderWidth * 2;
        int winH = availableDesktopHeight() - 4;

        oa.window = std::make_unique<Window>(
            appName, winX, winY, winW, winH);

        setupWindowCallbacks(*this, oa);

        oa.window->mount();
        oa.app->setup(
            oa.window->getContent(),
            oa.window->contentW(),
            oa.window->contentH());

        openApps.push_back(std::move(oa));
        updateActiveStates();
        markDirty();
    }

    void WindowManager::closeApp(const char *appName)
    {
        auto it = std::find_if(openApps.begin(), openApps.end(),
                               [&](const OpenApp &oa)
                               { return oa.name == appName; });
        if (it != openApps.end())
        {
            it->app->teardown();
            it->window->unmount();
            openApps.erase(it);
            updateActiveStates();
            markDirty();
        }
    }

    void WindowManager::focusApp(const char *appName)
    {
        auto it = std::find_if(openApps.begin(), openApps.end(),
                               [&](const OpenApp &oa)
                               { return oa.name == appName; });
        if (it != openApps.end() && it != openApps.end() - 1)
        {
            OpenApp tmp = std::move(*it);
            openApps.erase(it);
            openApps.push_back(std::move(tmp));
            markDirty();
        }
        updateActiveStates();
    }

    void WindowManager::minimizeApp(const char *appName)
    {
        auto it = std::find_if(openApps.begin(), openApps.end(),
                               [&](const OpenApp &oa)
                               { return oa.name == appName; });
        if (it != openApps.end())
        {
            it->window->setState(WindowState::Minimized);
            markDirty();
        }
    }

    void WindowManager::restoreApp(const char *appName)
    {
        auto it = std::find_if(openApps.begin(), openApps.end(),
                               [&](const OpenApp &oa)
                               { return oa.name == appName; });
        if (it != openApps.end())
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
        auto it = std::find_if(openApps.begin(), openApps.end(),
                               [&](const OpenApp &oa)
                               { return oa.name == appName; });
        if (it == openApps.end())
            return;

        int winX = Theme::WindowBorderWidth;
        int winY = Theme::DesktopY + 2;
        int winW = Theme::ScreenWidth - Theme::WindowBorderWidth * 2;
        int deskH = availableDesktopHeight();
        int winH = deskH - 4;

        switch (state)
        {
        case WindowState::Maximized:
            winX = 0;
            winY = Theme::DesktopY;
            winW = Theme::ScreenWidth;
            winH = deskH;
            break;
        case WindowState::TopHalf:
            winX = 0;
            winY = Theme::DesktopY;
            winW = Theme::ScreenWidth;
            winH = deskH / 2;
            break;
        case WindowState::BottomHalf:
            winX = 0;
            winY = Theme::DesktopY + deskH / 2;
            winW = Theme::ScreenWidth;
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

        it->window = std::make_unique<Window>(
            it->name.c_str(), winX, winY, winW, winH);
        it->window->setState(state);

        setupWindowCallbacks(*this, *it);

        it->window->mount();
        it->app->setup(
            it->window->getContent(),
            it->window->contentW(),
            it->window->contentH());

        updateActiveStates();
        markDirty();
    }

    bool WindowManager::isAppOpen(const char *appName) const
    {
        for (auto &oa : openApps)
        {
            if (oa.name == appName)
                return true;
        }
        return false;
    }

    OpenApp *WindowManager::getFocused()
    {
        if (openApps.empty())
            return nullptr;
        // find topmost non-minimized
        for (int i = (int)openApps.size() - 1; i >= 0; i--)
        {
            if (!openApps[i].window->isMinimized())
                return &openApps[i];
        }
        return nullptr;
    }

    void WindowManager::updateActiveStates()
    {
        auto *focused = getFocused();
        for (auto &oa : openApps)
        {
            oa.window->setActive(focused && &oa == focused);
        }
    }

    void WindowManager::draw()
    {
        auto &c = canvas();
        // desktop background
        c.fillRect(0, Theme::DesktopY, Theme::ScreenWidth, Theme::DesktopHeight, Theme::DesktopBg);

        // draw windows in z-order (back to front), skip minimized
        for (auto &oa : openApps)
        {
            if (!oa.window->isMinimized())
                oa.window->draw();
        }

        // overlay (taskbar, start menu)
        if (overlayDraw)
            overlayDraw();
    }

    void WindowManager::handleTouch(int px, int py)
    {
        // overlay gets first crack (taskbar, start menu)
        if (overlayTouch && overlayTouch(px, py))
            return;

        // hit-test windows top-down (back of vector = top), skip minimized
        for (int i = (int)openApps.size() - 1; i >= 0; i--)
        {
            if (openApps[i].window->isMinimized())
                continue;
            if (openApps[i].window->contains(px, py))
            {
                // focus this window if not already focused
                auto *focused = getFocused();
                if (&openApps[i] != focused)
                {
                    String name = openApps[i].name;
                    focusApp(name.c_str());
                }
                // top non-minimized window gets the touch
                auto *nowFocused = getFocused();
                if (nowFocused)
                    nowFocused->window->onTouch(px, py);
                return;
            }
        }
    }

    void WindowManager::handleTouchEnd(int px, int py)
    {
        if (overlayTouchEnd && overlayTouchEnd(px, py))
            return;

        // forward to focused (non-minimized) window
        auto *focused = getFocused();
        if (focused)
            focused->window->onTouchEnd(px, py);
    }

    void WindowManager::openPanel(const char *name, App *appInstance, int x, int y, int w, int h)
    {
        if (!appInstance)
            return;

        auto slot = std::make_unique<PanelSlot>();
        slot->name = name;
        slot->app.reset(appInstance);
        slot->container = std::make_unique<Container>();
        slot->container->setBounds(x, y, w, h);
        slot->container->mount();
        slot->app->setup(*slot->container, w, h);

        panelSlot = std::move(slot);
        markDirty();
    }

    void WindowManager::closePanel(const char *name)
    {
        if (panelSlot && panelSlot->name == name)
        {
            panelSlot->app->teardown();
            panelSlot->container->unmount();
            panelSlot.reset();
            markDirty();
        }
    }

    void WindowManager::tickTimers()
    {
        for (auto &oa : openApps)
        {
            if (oa.app->tickTimers())
                markDirty();
        }
        if (panelSlot && panelSlot->app->tickTimers())
            markDirty();
    }

    int WindowManager::availableDesktopHeight() const
    {
        if (keyboardVisible)
            return Theme::DesktopHeight - Theme::KeyboardHeight;
        return Theme::DesktopHeight;
    }

    void WindowManager::setKeyboardVisible(bool vis)
    {
        if (vis == keyboardVisible)
            return;
        keyboardVisible = vis;
        relayoutWindows();
        markDirty();
    }

    void WindowManager::relayoutWindows()
    {
        int winX = Theme::WindowBorderWidth;
        int winY = Theme::DesktopY + 2;
        int winW = Theme::ScreenWidth - Theme::WindowBorderWidth * 2;
        int winH = availableDesktopHeight() - 4;

        for (auto &oa : openApps)
        {
            oa.app->teardown();
            oa.window->unmount();

            WindowState prevState = oa.window->getState();

            oa.window = std::make_unique<Window>(
                oa.name.c_str(), winX, winY, winW, winH);
            oa.window->setState(prevState);

            setupWindowCallbacks(*this, oa);

            oa.window->mount();
            oa.app->setup(
                oa.window->getContent(),
                oa.window->contentW(),
                oa.window->contentH());
        }
        updateActiveStates();
    }

} // namespace UI

#endif
