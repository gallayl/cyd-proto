#pragma once

#include <string>
#include <vector>
#include "button.h"
#include "../Renderer.h"
#include "../Theme.h"

namespace UI
{

class ErrorPopup
{
public:
    void show(const char *message)
    {
        _message = message;
        _visible = true;
        if (!_okBtn.isMounted())
            _okBtn.mount();
        _okBtn.setCallback([this]() { hide(); });
        markDirty();
    }

    void hide()
    {
        _visible = false;
        _okBtn.setPressed(false);
        markDirty();
    }

    bool isVisible() const
    {
        return _visible;
    }

    void draw()
    {
        if (!_visible)
            return;

        auto &c = canvas();
        int oY = UI::stripOffsetY();

        constexpr int dlgW = 180;
        constexpr int padX = 8;
        constexpr int padY = 6;
        constexpr int titleH = Theme::TitleBarHeight;
        constexpr int btnW = 50;
        constexpr int btnH = 20;
        constexpr int btnPad = 8;
        int textAreaW = dlgW - padX * 2;

        c.setTextSize(1);
        int lineH = c.fontHeight();
        auto lines = wrapText(c, _message, textAreaW);
        int textH = lines.size() * lineH;

        int dlgH = titleH + padY + textH + padY + btnH + btnPad;
        int dlgX = (Theme::ScreenWidth() - dlgW) / 2;
        int dlgY = (Theme::ScreenHeight() - dlgH) / 2;

        // store absolute coords for hit-testing
        _bounds[0] = dlgX;
        _bounds[1] = dlgY;
        _bounds[2] = dlgW;
        _bounds[3] = dlgH;

        // button uses absolute coords (its own draw applies strip offset)
        int btnX = dlgX + (dlgW - btnW) / 2;
        int btnY = dlgY + dlgH - btnH - btnPad / 2;
        _okBtn.setBounds(btnX, btnY, btnW, btnH);

        // offset dlgY for drawing
        dlgY -= oY;

        // outer border (3D raised)
        c.fillRect(dlgX, dlgY, dlgW, dlgH, Theme::WindowBg);
        c.drawRect(dlgX, dlgY, dlgW, dlgH, Theme::WindowBorder);
        c.drawFastHLine(dlgX + 1, dlgY + 1, dlgW - 2, Theme::ButtonHighlight);
        c.drawFastVLine(dlgX + 1, dlgY + 1, dlgH - 2, Theme::ButtonHighlight);
        c.drawFastHLine(dlgX + 1, dlgY + dlgH - 2, dlgW - 2, Theme::ButtonShadow);
        c.drawFastVLine(dlgX + dlgW - 2, dlgY + 1, dlgH - 2, Theme::ButtonShadow);

        // title bar
        int tbX = dlgX + 2;
        int tbY = dlgY + 2;
        int tbW = dlgW - 4;
        c.fillRect(tbX, tbY, tbW, titleH, Theme::TitleBarActive);
        c.setTextColor(Theme::TitleTextActive, Theme::TitleBarActive);
        c.setTextSize(1);
        int16_t th = c.fontHeight();
        c.setCursor(tbX + 4, tbY + (titleH - th) / 2);
        c.print("Error");

        // message text
        int textX = dlgX + padX;
        int textY = dlgY + titleH + padY + 2;
        c.setTextColor(Theme::TextColor, Theme::WindowBg);
        c.setTextSize(1);
        for (size_t i = 0; i < lines.size(); i++)
        {
            c.setCursor(textX, textY + i * lineH);
            c.print(lines[i].c_str());
        }

        // Ok button
        _okBtn.setLabel("Ok");
        _okBtn.setTextSize(1);
        _okBtn.setBackgroundColor(Theme::ButtonFace);
        _okBtn.setTextColor(Theme::TextColor, Theme::ButtonFace);
        _okBtn.setBorderColors(Theme::ButtonHighlight, Theme::ButtonShadow);
        _okBtn.draw();
    }

    bool handleTouch(int px, int py)
    {
        if (!_visible)
            return false;

        if (inBounds(px, py))
        {
            _okBtn.onTouch(px, py);
            return true;
        }

        hide();
        return true;
    }

    bool handleTouchEnd(int px, int py)
    {
        if (!_visible)
            return false;

        if (inBounds(px, py))
        {
            _okBtn.onTouchEnd(px, py);
            return true;
        }

        hide();
        return true;
    }

private:
    bool _visible{false};
    std::string _message;
    Button _okBtn;
    int _bounds[4]{0, 0, 0, 0};

    bool inBounds(int px, int py) const
    {
        return px >= _bounds[0] && px < _bounds[0] + _bounds[2] && py >= _bounds[1] && py < _bounds[1] + _bounds[3];
    }

    static std::vector<std::string> wrapText(LGFX_Sprite &c, const std::string &text, int maxWidth)
    {
        std::vector<std::string> lines;
        int start = 0;
        int len = text.length();

        while (start < len)
        {
            size_t nl = text.find('\n', start);
            int segEnd = (nl != std::string::npos) ? (int)nl : len;
            std::string segment = text.substr(start, segEnd - start);

            if (c.textWidth(segment.c_str()) <= maxWidth)
            {
                lines.push_back(segment);
            }
            else
            {
                std::string line;
                int i = 0;
                while (i < (int)segment.length())
                {
                    size_t wordEnd = segment.find(' ', i);
                    if (wordEnd == std::string::npos)
                        wordEnd = segment.length();
                    std::string word = segment.substr(i, wordEnd - i);

                    std::string candidate = line.length() > 0 ? line + " " + word : word;
                    if (c.textWidth(candidate.c_str()) > maxWidth && line.length() > 0)
                    {
                        lines.push_back(line);
                        line = word;
                    }
                    else
                    {
                        line = candidate;
                    }
                    i = wordEnd + 1;
                }
                if (line.length() > 0)
                    lines.push_back(line);
            }

            start = segEnd + 1;
        }

        if (lines.empty())
            lines.push_back("");

        return lines;
    }
};

inline ErrorPopup &errorPopup()
{
    static ErrorPopup ep;
    return ep;
}

} // namespace UI
