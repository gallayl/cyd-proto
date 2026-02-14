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

    void WindowManager::openApp(const char *appName)
    {
        // if already open, just focus
        for (auto &oa : openApps)
        {
            if (oa.name == appName)
            {
                focusApp(appName);
                return;
            }
        }

        // find factory
        for (auto &entry : appRegistry())
        {
            if (String(entry.name) == appName)
            {
                OpenApp oa;
                oa.name = appName;
                oa.app.reset(entry.factory());

                int winX = Theme::WindowBorderWidth;
                int winY = Theme::DesktopY + 2;
                int winW = Theme::ScreenWidth - Theme::WindowBorderWidth * 2;
                int winH = availableDesktopHeight() - 4;

                oa.window = std::make_unique<Window>(
                    appName, winX, winY, winW, winH);

                oa.window->setCloseCallback([this, name = String(appName)]()
                                            { closeApp(name.c_str()); });

                oa.window->mount();
                oa.app->setup(
                    oa.window->getContent(),
                    oa.window->contentW(),
                    oa.window->contentH());

                openApps.push_back(std::move(oa));
                updateActiveStates();
                markDirty();
                return;
            }
        }
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
        return &openApps.back();
    }

    void WindowManager::updateActiveStates()
    {
        for (size_t i = 0; i < openApps.size(); i++)
        {
            openApps[i].window->setActive(i == openApps.size() - 1);
        }
    }

    void WindowManager::draw()
    {
        auto &c = canvas();
        // desktop background
        c.fillRect(0, Theme::DesktopY, Theme::ScreenWidth, Theme::DesktopHeight, Theme::DesktopBg);

        // draw windows in z-order (back to front)
        for (auto &oa : openApps)
        {
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

        // hit-test windows top-down (back of vector = top)
        for (int i = (int)openApps.size() - 1; i >= 0; i--)
        {
            if (openApps[i].window->contains(px, py))
            {
                // focus this window if not already focused
                if (i != (int)openApps.size() - 1)
                {
                    String name = openApps[i].name;
                    focusApp(name.c_str());
                }
                // top window gets the touch
                openApps.back().window->onTouch(px, py);
                return;
            }
        }
    }

    void WindowManager::handleTouchEnd(int px, int py)
    {
        if (overlayTouchEnd && overlayTouchEnd(px, py))
            return;

        // forward to focused window
        if (!openApps.empty())
        {
            openApps.back().window->onTouchEnd(px, py);
        }
    }

    void WindowManager::tickTimers()
    {
        for (auto &oa : openApps)
        {
            if (oa.app->tickTimers())
                markDirty();
        }
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

            oa.window = std::make_unique<Window>(
                oa.name.c_str(), winX, winY, winW, winH);

            oa.window->setCloseCallback([this, name = String(oa.name)]()
                                        { closeApp(name.c_str()); });

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
