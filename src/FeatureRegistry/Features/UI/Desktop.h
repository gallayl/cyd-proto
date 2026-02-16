#pragma once

#include "WindowManager.h"
#include "elements/keyboard.h"
#include "elements/textfield.h"
#include "elements/error_popup.h"

namespace UI
{

class Desktop
{
public:
    void init()
    {
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
        renderStrips([this]()
                     { windowManager().draw(); });
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

    void toggleKeyboard()
    {
        keyboard.toggle();
        windowManager().setKeyboardVisible(keyboard.isVisible());
    }

private:
    Keyboard keyboard;
    std::function<void(char)> keyConsumer;

    void drawOverlays()
    {
        keyboard.draw();
        auto *panel = windowManager().getPanelSlot();
        if (panel)
        {
            auto &c = canvas();
            int tbY = Theme::TaskbarY() - stripOffsetY();
            c.fillRect(0, tbY, Theme::ScreenWidth(), Theme::TaskbarHeight, Theme::TaskbarBg);
            c.drawFastHLine(0, tbY, Theme::ScreenWidth(), Theme::ButtonHighlight);
            panel->container->draw();
        }
        windowManager().drawPopups();
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
        // popups (start menu etc.) get next priority
        if (windowManager().handlePopupTouch(px, py))
            return true;
        if (keyboard.handleTouch(px, py))
            return true;
        // panel (taskbar) touch
        auto *panel = windowManager().getPanelSlot();
        if (panel && py >= Theme::TaskbarY())
        {
            panel->container->handleTouch(px, py);
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
        if (windowManager().handlePopupTouchEnd(px, py))
            return true;
        if (keyboard.handleTouchEnd(px, py))
            return true;
        auto *panel = windowManager().getPanelSlot();
        if (panel && py >= Theme::TaskbarY())
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
