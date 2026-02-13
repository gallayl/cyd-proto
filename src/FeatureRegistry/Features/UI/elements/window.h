#pragma once

#include <functional>
#include "container.h"
#include "label.h"
#include "button.h"
#include "../Renderer.h"
#include "../Theme.h"

namespace UI
{

    class Window : public Element
    {
    public:
        using CloseCallback = std::function<void()>;

        Window(const String &title, int ix, int iy, int iw, int ih)
            : titleText(title),
              closeBtn("x", 0, 0, Theme::CloseButtonSize, Theme::CloseButtonSize),
              titleLabel(title, 0, 0, 0, Theme::TitleBarHeight),
              content()
        {
            setBounds(ix, iy, iw, ih);
            titleLabel.setTextSize(1);
            closeBtn.setTextSize(1);
            closeBtn.setBackgroundColor(Theme::ButtonFace);
            closeBtn.setTextColor(Theme::TextColor, Theme::ButtonFace);
            closeBtn.setBorderColors(Theme::ButtonHighlight, Theme::ButtonShadow);
        }

        void setCloseCallback(CloseCallback cb)
        {
            closeBtn.setCallback(std::move(cb));
        }

        void setActive(bool a)
        {
            active = a;
        }
        bool isActive() const { return active; }

        const String &getTitle() const { return titleText; }

        Container &getContent() { return content; }

        void mount() override
        {
            if (mounted)
                return;
            Element::mount();
            titleLabel.mount();
            closeBtn.mount();
            content.mount();
        }

        void unmount() override
        {
            if (!mounted)
                return;
            content.unmount();
            closeBtn.unmount();
            titleLabel.unmount();
            Element::unmount();
        }

        void draw() override
        {
            if (!mounted)
                return;
            auto &c = canvas();

            int bw = Theme::WindowBorderWidth;

            // outer border (3D raised)
            c.drawRect(x, y, width, height, Theme::WindowBorder);
            c.drawFastHLine(x + 1, y + 1, width - 2, Theme::ButtonHighlight);
            c.drawFastVLine(x + 1, y + 1, height - 2, Theme::ButtonHighlight);
            c.drawFastHLine(x + 1, y + height - 2, width - 2, Theme::ButtonShadow);
            c.drawFastVLine(x + width - 2, y + 1, height - 2, Theme::ButtonShadow);

            // title bar background
            uint16_t tbColor = active ? Theme::TitleBarActive : Theme::TitleBarInactive;
            uint16_t tbText = active ? Theme::TitleTextActive : Theme::TitleTextInactive;
            int tbX = x + bw;
            int tbY = y + bw;
            int tbW = width - bw * 2;
            int tbH = Theme::TitleBarHeight;
            c.fillRect(tbX, tbY, tbW, tbH, tbColor);

            // title text
            titleLabel.setBounds(tbX + 2, tbY, tbW - Theme::CloseButtonSize - 6, tbH);
            titleLabel.setTextColor(tbText, tbColor);
            titleLabel.setAlign(lgfx::textdatum_t::top_left);
            titleLabel.draw();

            // close button
            int cbX = x + width - bw - Theme::CloseButtonSize - 2;
            int cbY = tbY + (tbH - Theme::CloseButtonSize) / 2;
            closeBtn.setBounds(cbX, cbY, Theme::CloseButtonSize, Theme::CloseButtonSize);
            closeBtn.draw();

            // content area background
            int cX = x + bw;
            int cY = tbY + tbH;
            int cW = width - bw * 2;
            int cH = height - bw * 2 - tbH;
            c.fillRect(cX, cY, cW, cH, Theme::WindowBg);

            // draw content children
            content.setBounds(cX, cY, cW, cH);
            content.draw();
        }

        void onTouch(int px, int py) override
        {
            if (!mounted)
                return;
            int bw = Theme::WindowBorderWidth;
            int cbX = x + width - bw - Theme::CloseButtonSize - 2;
            int cbY = y + bw + (Theme::TitleBarHeight - Theme::CloseButtonSize) / 2;
            if (closeBtn.contains(px, py))
            {
                closeBtn.onTouch(px, py);
                return;
            }
            int cX, cY, cW, cH;
            content.getBounds(cX, cY, cW, cH);
            if (px >= cX && px < cX + cW && py >= cY && py < cY + cH)
            {
                content.handleTouch(px, py);
            }
        }

        void onTouchEnd(int px, int py) override
        {
            if (!mounted)
                return;
            closeBtn.onTouchEnd(px, py);
            content.handleTouchEnd(px, py);
        }

        int contentX() const { return x + Theme::WindowBorderWidth; }
        int contentY() const { return y + Theme::WindowBorderWidth + Theme::TitleBarHeight; }
        int contentW() const { return width - Theme::WindowBorderWidth * 2; }
        int contentH() const { return height - Theme::WindowBorderWidth * 2 - Theme::TitleBarHeight; }

    private:
        String titleText;
        bool active{true};
        Button closeBtn;
        Label titleLabel;
        Container content;
    };

} // namespace UI
