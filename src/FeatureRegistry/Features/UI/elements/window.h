#pragma once

#include <functional>
#include "container.h"
#include "scrollable.h"
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
              scrollableContent()
        {
            setBounds(ix, iy, iw, ih);
            scrollableContent.setBounds(contentX(), contentY(), contentW(), contentH());
            scrollableContent.getContent().setBounds(contentX(), contentY(), contentW(), contentH());
            scrollableContent.setAutoContentHeight(true);
            scrollableContent.setThinScrollbar(true);
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

        Container &getContent() { return scrollableContent.getContent(); }

        void mount() override
        {
            if (mounted)
                return;
            Element::mount();
            titleLabel.mount();
            closeBtn.mount();
            scrollableContent.mount();
        }

        void unmount() override
        {
            if (!mounted)
                return;
            scrollableContent.unmount();
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
            titleLabel.setAlign(TextAlign::LEFT);
            titleLabel.draw();

            // close button
            int cbX = x + width - bw - Theme::CloseButtonSize - 2;
            int cbY = tbY + (tbH - Theme::CloseButtonSize) / 2;
            closeBtn.setBounds(cbX, cbY, Theme::CloseButtonSize, Theme::CloseButtonSize);
            closeBtn.draw();

            // content area background
            int cX = contentX();
            int cY = contentY();
            int cW = contentW();
            int cH = contentH();
            c.fillRect(cX, cY, cW, cH, Theme::WindowBg);

            // draw scrollable content
            scrollableContent.setBounds(cX, cY, cW, cH);
            scrollableContent.draw();
        }

        void onTouch(int px, int py) override
        {
            if (!mounted)
                return;
            if (closeBtn.contains(px, py))
            {
                closeBtn.onTouch(px, py);
                return;
            }
            int cX = contentX();
            int cY = contentY();
            int cW = contentW();
            int cH = contentH();
            if (px >= cX && px < cX + cW && py >= cY && py < cY + cH)
            {
                scrollableContent.onTouch(px, py);
            }
        }

        void onTouchEnd(int px, int py) override
        {
            if (!mounted)
                return;
            closeBtn.onTouchEnd(px, py);
            scrollableContent.onTouchEnd(px, py);
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
        ScrollableContainer scrollableContent;
    };

} // namespace UI
