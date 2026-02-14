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

        void setAutoContentHeight(bool enabled) { autoHeight = enabled; }
        void setThinScrollbar(bool enabled) { thinScrollbar = enabled; }

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

        void autoContentHeightFromChildren()
        {
            int maxBottom = y;
            for (auto &child : content.getChildren())
            {
                int cx, cy, cw, ch;
                child->getBounds(cx, cy, cw, ch);
                int bottom = cy + ch;
                if (bottom > maxBottom)
                    maxBottom = bottom;
            }
            contentHeight = maxBottom - y;
        }

        void draw() override
        {
            if (!mounted)
                return;
            auto &c = canvas();

            if (autoHeight)
                autoContentHeightFromChildren();

            int sbW = scrollbarWidth();
            bool needsScroll = contentHeight > height;
            int viewW = needsScroll ? width - sbW : width;

            // clip to viewport
            c.setClipRect(x, y, viewW, height);

            // temporarily offset children by -scrollOffset so the clip rect
            // hides elements above/below the visible area
            const auto &children = content.getChildren();
            for (auto &child : children)
            {
                int cx, cy, cw, ch;
                child->getBounds(cx, cy, cw, ch);
                child->setBounds(cx, cy - scrollOffset, cw, ch);
            }

            content.setBounds(x, y, viewW, contentHeight);
            content.draw();

            // restore original positions
            for (auto &child : children)
            {
                int cx, cy, cw, ch;
                child->getBounds(cx, cy, cw, ch);
                child->setBounds(cx, cy + scrollOffset, cw, ch);
            }

            c.clearClipRect();

            // scrollbar
            if (needsScroll)
            {
                if (thinScrollbar)
                    drawThinScrollbar(c);
                else
                    drawScrollbar(c);
            }
        }

        void onTouch(int px, int py) override
        {
            if (!mounted)
                return;

            int sbW = scrollbarWidth();

            // continuous thumb drag
            if (draggingThumb)
            {
                int trackH = thinScrollbar ? height : (height - 2 * sbW);
                int maxScroll = contentHeight - height;
                if (maxScroll > 0 && trackH > 0)
                {
                    int delta = py - dragStartY;
                    int thumbH = (height * trackH) / contentHeight;
                    if (thumbH < 10)
                        thumbH = 10;
                    int thumbTrack = trackH - thumbH;
                    if (thumbTrack > 0)
                    {
                        scrollOffset = dragStartOffset + (delta * maxScroll) / thumbTrack;
                        clampScroll();
                    }
                }
                return;
            }

            // continuous content drag
            if (draggingContent)
            {
                int delta = lastTouchY - py;
                scrollOffset += delta;
                clampScroll();
                lastTouchY = py;
                return;
            }

            // first touch
            bool needsScroll = contentHeight > height;
            int sbX = x + width - sbW;

            if (needsScroll && px >= sbX)
            {
                draggingThumb = true;
                dragStartY = py;
                dragStartOffset = scrollOffset;
                return;
            }

            // forward to content (adjust for scroll offset)
            int adjY = py + scrollOffset;
            content.handleTouch(px, adjY);

            if (needsScroll)
            {
                draggingContent = true;
                lastTouchY = py;
            }
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

    private:
        Container content;
        int contentHeight{0};
        int scrollOffset{0};
        bool draggingThumb{false};
        bool draggingContent{false};
        int dragStartY{0};
        int dragStartOffset{0};
        int lastTouchY{0};
        bool autoHeight{false};
        bool thinScrollbar{false};

        int scrollbarWidth() const
        {
            return thinScrollbar ? Theme::ThinScrollbarWidth : Theme::ScrollbarWidth;
        }

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

        void drawThinScrollbar(LGFX_Sprite &c)
        {
            int sbW = Theme::ThinScrollbarWidth;
            int sbX = x + width - sbW;
            int sbY = y;
            int sbH = height;

            // thin track
            c.fillRect(sbX, sbY, sbW, sbH, Theme::ScrollTrack);

            int maxScroll = contentHeight - height;
            if (maxScroll <= 0)
                return;

            // thumb proportional to visible area
            int thumbH = (height * sbH) / contentHeight;
            if (thumbH < 8)
                thumbH = 8;
            int thumbTrack = sbH - thumbH;
            int thumbY = sbY + (thumbTrack * scrollOffset) / maxScroll;

            c.fillRect(sbX + 1, thumbY, sbW - 2, thumbH, Theme::ButtonShadow);
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
