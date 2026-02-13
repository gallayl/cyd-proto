#pragma once

#include "container.h"
#include "../Renderer.h"
#include "../Theme.h"

namespace UI
{

    class ScrollableContainer : public Element
    {
    public:
        ScrollableContainer() = default;

        void setContentHeight(int h) { contentHeight = h; }
        int getContentHeight() const { return contentHeight; }
        int getScrollOffset() const { return scrollOffset; }

        void addChild(std::unique_ptr<Element> child)
        {
            content.addChild(std::move(child));
        }

        Container &getContent() { return content; }

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

            bool needsScroll = contentHeight > height;
            int viewW = needsScroll ? width - Theme::ScrollbarWidth : width;

            // clip to viewport
            c.setClipRect(x, y, viewW, height);

            // position content offset
            content.setBounds(x, y - scrollOffset, viewW, contentHeight);
            content.draw();

            c.clearClipRect();

            // scrollbar
            if (needsScroll)
            {
                drawScrollbar(c);
            }
        }

        void onTouch(int px, int py) override
        {
            if (!mounted)
                return;

            bool needsScroll = contentHeight > height;
            int sbX = x + width - Theme::ScrollbarWidth;

            if (needsScroll && px >= sbX)
            {
                // scrollbar interaction
                draggingThumb = true;
                dragStartY = py;
                dragStartOffset = scrollOffset;
                return;
            }

            // forward to content (adjust for scroll offset)
            int adjY = py + scrollOffset;
            content.handleTouch(px, adjY);

            // start drag tracking for content area scrolling
            draggingContent = true;
            lastTouchY = py;
        }

        void onTouchEnd(int px, int py) override
        {
            if (!mounted)
                return;

            if (draggingThumb)
            {
                draggingThumb = false;
                return;
            }

            draggingContent = false;
            int adjY = py + scrollOffset;
            content.handleTouchEnd(px, adjY);
        }

        void handleDrag(int px, int py)
        {
            if (!mounted)
                return;

            if (draggingThumb)
            {
                int trackH = height - 2 * Theme::ScrollbarWidth;
                int maxScroll = contentHeight - height;
                if (maxScroll <= 0 || trackH <= 0)
                    return;

                int delta = py - dragStartY;
                int thumbH = (height * trackH) / contentHeight;
                if (thumbH < 10)
                    thumbH = 10;
                int thumbTrack = trackH - thumbH;
                if (thumbTrack <= 0)
                    return;

                scrollOffset = dragStartOffset + (delta * maxScroll) / thumbTrack;
                clampScroll();
                return;
            }

            if (draggingContent)
            {
                int delta = lastTouchY - py;
                scrollOffset += delta;
                clampScroll();
                lastTouchY = py;
            }
        }

    private:
        Container content;
        int contentHeight{0};
        int scrollOffset{0};
        bool draggingThumb{false};
        bool draggingContent{false};
        int dragStartY{0};
        int dragStartOffset{0};
        int lastTouchY{0};

        void clampScroll()
        {
            int maxScroll = contentHeight - height;
            if (maxScroll < 0)
                maxScroll = 0;
            if (scrollOffset < 0)
                scrollOffset = 0;
            if (scrollOffset > maxScroll)
                scrollOffset = maxScroll;
        }

        void drawScrollbar(LGFX_Sprite &c)
        {
            int sbX = x + width - Theme::ScrollbarWidth;
            int sbY = y;
            int sbW = Theme::ScrollbarWidth;
            int sbH = height;

            // track background
            c.fillRect(sbX, sbY, sbW, sbH, Theme::ScrollTrack);
            c.drawRect(sbX, sbY, sbW, sbH, Theme::ButtonShadow);

            // up arrow button
            int arrowH = Theme::ScrollbarWidth;
            drawScrollButton(c, sbX, sbY, sbW, arrowH, true);

            // down arrow button
            drawScrollButton(c, sbX, sbY + sbH - arrowH, sbW, arrowH, false);

            // thumb
            int trackH = sbH - 2 * arrowH;
            if (trackH <= 0)
                return;
            int maxScroll = contentHeight - height;
            if (maxScroll <= 0)
                return;

            int thumbH = (height * trackH) / contentHeight;
            if (thumbH < 10)
                thumbH = 10;
            int thumbTrack = trackH - thumbH;
            int thumbY = sbY + arrowH + (thumbTrack * scrollOffset) / maxScroll;

            // thumb body
            c.fillRect(sbX + 1, thumbY, sbW - 2, thumbH, Theme::ButtonFace);
            // 3D borders on thumb
            c.drawFastHLine(sbX + 1, thumbY, sbW - 2, Theme::ButtonHighlight);
            c.drawFastVLine(sbX + 1, thumbY, thumbH, Theme::ButtonHighlight);
            c.drawFastHLine(sbX + 1, thumbY + thumbH - 1, sbW - 2, Theme::ButtonShadow);
            c.drawFastVLine(sbX + sbW - 2, thumbY, thumbH, Theme::ButtonShadow);
        }

        static void drawScrollButton(LGFX_Sprite &c, int bx, int by, int bw, int bh, bool up)
        {
            c.fillRect(bx, by, bw, bh, Theme::ButtonFace);
            c.drawFastHLine(bx, by, bw, Theme::ButtonHighlight);
            c.drawFastVLine(bx, by, bh, Theme::ButtonHighlight);
            c.drawFastHLine(bx, by + bh - 1, bw, Theme::ButtonShadow);
            c.drawFastVLine(bx + bw - 1, by, bh, Theme::ButtonShadow);

            // small arrow triangle
            int cx = bx + bw / 2;
            int cy = by + bh / 2;
            if (up)
            {
                c.fillTriangle(cx, cy - 2, cx - 3, cy + 2, cx + 3, cy + 2, Theme::TextColor);
            }
            else
            {
                c.fillTriangle(cx, cy + 2, cx - 3, cy - 2, cx + 3, cy - 2, Theme::TextColor);
            }
        }
    };

} // namespace UI
