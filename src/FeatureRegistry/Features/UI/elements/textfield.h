#pragma once

#include <functional>
#include <Arduino.h>
#include "container.h"
#include "../Renderer.h"
#include "../Theme.h"

namespace UI
{
// forward declaration — implemented in Desktop via a global callback
using KeyboardFocusRequestFn = std::function<void(std::function<void(char)>)>;
using KeyboardBlurFn = std::function<void()>;

namespace detail
{
inline KeyboardFocusRequestFn &keyboardFocusRequest()
{
    static KeyboardFocusRequestFn fn;
    return fn;
}
inline KeyboardBlurFn &keyboardBlur()
{
    static KeyboardBlurFn fn;
    return fn;
}
} // namespace detail

inline void setKeyboardFocusHandler(KeyboardFocusRequestFn fn)
{
    detail::keyboardFocusRequest() = std::move(fn);
}
inline void setKeyboardBlurHandler(KeyboardBlurFn fn)
{
    detail::keyboardBlur() = std::move(fn);
}

inline void requestKeyboardFocus(std::function<void(char)> consumer)
{
    if (detail::keyboardFocusRequest())
        detail::keyboardFocusRequest()(std::move(consumer));
}

inline void requestKeyboardBlur()
{
    if (detail::keyboardBlur())
        detail::keyboardBlur()();
}

class TextField : public Element
{
public:
    using ChangeCb = std::function<void(const String &)>;

    TextField(const String &txt = String(), int ix = 0, int iy = 0, int iw = 0, int ih = 0) : text(txt)
    {
        setBounds(ix, iy, iw, ih);
        cursorPos = text.length();
    }

    void setText(const String &t)
    {
        text = t;
        cursorPos = text.length();
    }
    const String &getText() const
    {
        return text;
    }

    void setMaxLength(int m)
    {
        maxLen = m;
    }
    void setOnChange(ChangeCb cb)
    {
        onChange = std::move(cb);
    }
    void setPlaceholder(const String &p)
    {
        placeholder = p;
    }

    void focus()
    {
        focused = true;
        requestKeyboardFocus([this](char ch) { handleKey(ch); });
    }

    void blur()
    {
        focused = false;
    }

    void draw() override
    {
        if (!mounted)
            return;
        auto &c = canvas();

        // sunken 3D border
        c.fillRect(drawX(), drawY(), width, height, Theme::TextFieldBg);
        c.drawFastHLine(drawX(), drawY(), width, Theme::ButtonShadow);
        c.drawFastVLine(drawX(), drawY(), height, Theme::ButtonShadow);
        c.drawFastHLine(drawX() + 1, drawY() + 1, width - 2, Theme::ButtonDarkShadow);
        c.drawFastVLine(drawX() + 1, drawY() + 1, height - 2, Theme::ButtonDarkShadow);
        c.drawFastHLine(drawX(), drawY() + height - 1, width, Theme::ButtonHighlight);
        c.drawFastVLine(drawX() + width - 1, drawY(), height, Theme::ButtonHighlight);
        c.drawFastHLine(drawX() + 1, drawY() + height - 2, width - 2, Theme::ButtonFace);
        c.drawFastVLine(drawX() + width - 2, drawY() + 1, height - 2, Theme::ButtonFace);

        int innerX = drawX() + 3;
        int innerW = width - 6;

        c.setTextSize(1);
        int16_t th = c.fontHeight();
        int ty = drawY() + (height - th) / 2;

        if (text.length() == 0 && !focused && placeholder.length() > 0)
        {
            c.setTextColor(Theme::ButtonShadow, Theme::TextFieldBg);
            c.setCursor(innerX, ty);
            c.print(placeholder);
        }
        else
        {
            c.setTextColor(Theme::TextColor, Theme::TextFieldBg);

            // compute visible portion of text
            int tw = c.textWidth(text);
            int offset = 0;
            if (tw > innerW)
                offset = tw - innerW;

            c.setClipRect(innerX, drawY() + 2, innerW, height - 4);
            c.setCursor(innerX - offset, ty);
            c.print(text);
            c.clearClipRect();

            // cursor
            if (focused)
            {
                String beforeCursor = text.substring(0, cursorPos);
                int cx = innerX + c.textWidth(beforeCursor) - offset;
                if (cx >= innerX && cx < innerX + innerW)
                {
                    c.drawFastVLine(cx, drawY() + 3, height - 6, Theme::TextColor);
                }
            }
        }
    }

    void onTouch(int px, int py) override
    {
        if (contains(px, py))
        {
            if (!focused)
                focus();
        }
    }

    void onTouchEnd(int px, int py) override
    {
        // nothing needed
    }

    void handleKey(char ch)
    {
        if (ch == '\b')
        {
            // backspace
            if (cursorPos > 0 && text.length() > 0)
            {
                text = text.substring(0, cursorPos - 1) + text.substring(cursorPos);
                cursorPos--;
                if (onChange)
                    onChange(text);
                markDirty();
            }
        }
        else if (ch == '\n' || ch == '\r')
        {
            // enter — blur
            blur();
            requestKeyboardBlur();
            markDirty();
        }
        else
        {
            if (maxLen > 0 && (int)text.length() >= maxLen)
                return;
            text = text.substring(0, cursorPos) + String(ch) + text.substring(cursorPos);
            cursorPos++;
            if (onChange)
                onChange(text);
            markDirty();
        }
    }

private:
    String text;
    String placeholder;
    int cursorPos{0};
    int maxLen{0};
    bool focused{false};
    ChangeCb onChange;
};

} // namespace UI
