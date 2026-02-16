#pragma once

#include "container.h"
#include "../Renderer.h"
#include "../Theme.h"

namespace UI
{

    class GroupBox : public Element
    {
    public:
        GroupBox(const String &lbl = String(), int ix = 0, int iy = 0, int iw = 0, int ih = 0)
            : label(lbl)
        {
            setBounds(ix, iy, iw, ih);
            updateContentBounds();
        }

        void setLabel(const String &lbl) { label = lbl; }

        Container &getContent() { return content; }

        void addChild(std::unique_ptr<Element> child) { content.addChild(std::move(child)); }

        void setBounds(int ix, int iy, int iw, int ih)
        {
            Element::setBounds(ix, iy, iw, ih);
            updateContentBounds();
        }

        void mount() override
        {
            if (mounted)
                return;
            Element::mount();
            content.mount();
        }

        void unmount() override
        {
            if (!mounted)
                return;
            content.unmount();
            Element::unmount();
        }

        void draw() override
        {
            if (!mounted)
                return;
            auto &c = canvas();

            int borderY = drawY() + Theme::GroupBoxTopPad / 2;
            int borderH = height - Theme::GroupBoxTopPad / 2;

            // etched border: dark outer, light inner offset by 1px
            c.drawRect(drawX() + 1, borderY, width - 2, borderH, Theme::ButtonShadow);
            c.drawRect(drawX() + 2, borderY + 1, width - 2, borderH, Theme::ButtonHighlight);

            // label background + text
            if (label.length() > 0)
            {
                c.setTextSize(1);
                int16_t tw = c.textWidth(label);
                int lx = drawX() + Theme::GroupBoxPad + 4;

                c.fillRect(lx - 2, borderY - 1, tw + 4, 3, Theme::WindowBg);
                c.setTextColor(Theme::TextColor, Theme::WindowBg);
                c.setCursor(lx, drawY());
                c.print(label);
            }

            // draw children
            content.draw();
        }

        void onTouch(int px, int py) override
        {
            content.handleTouch(px, py);
        }

        void onTouchEnd(int px, int py) override
        {
            content.handleTouchEnd(px, py);
        }

        int contentX() const { return x + Theme::GroupBoxPad; }
        int contentY() const { return y + Theme::GroupBoxTopPad; }
        int contentW() const { return width - Theme::GroupBoxPad * 2; }
        int contentH() const { return height - Theme::GroupBoxTopPad - Theme::GroupBoxPad; }

    private:
        String label;
        Container content;

        void updateContentBounds()
        {
            content.setBounds(contentX(), contentY(), contentW(), contentH());
        }
    };

} // namespace UI
