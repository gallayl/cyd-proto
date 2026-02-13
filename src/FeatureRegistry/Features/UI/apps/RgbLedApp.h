#pragma once

#include "../App.h"
#include "../elements/container.h"
#include "../elements/label.h"
#include "../elements/button.h"
#include "../Renderer.h"
#include <memory>
#include "../Theme.h"
#include "../../../../hw/RgbLed.h"

namespace UI
{

    class RgbLedApp : public App
    {
    public:
        const char *name() const override { return "RGB LED"; }

        void setup(Container &cont, int w, int h) override
        {
            contentRef = &cont;
            contentW = w;
            contentH = h;

            int baseY = 0;
            int cx, cy, cw, ch;
            cont.getBounds(cx, cy, cw, ch);

            // color preview at top
            previewY = cy + 4;
            previewX = cx + 4;
            previewW = w - 8;
            previewH = 30;
            baseY = 38;

            // Red row
            addColorRow(cont, cx, cy + baseY, w, "R", r, TFT_RED);
            baseY += 34;
            // Green row
            addColorRow(cont, cx, cy + baseY, w, "G", g, TFT_GREEN);
            baseY += 34;
            // Blue row
            addColorRow(cont, cx, cy + baseY, w, "B", b, 0x001F);
            baseY += 34;

            // Off button
            auto offBtn = std::make_unique<Button>("Off", cx + 4, cy + baseY, w - 8, 22);
            offBtn->setBackgroundColor(Theme::ButtonFace);
            offBtn->setTextColor(Theme::TextColor, Theme::ButtonFace);
            offBtn->setCallback([this]()
                                {
                r = 0; g = 0; b = 0;
                applyColor(); });
            cont.addChild(std::move(offBtn));
        }

        void loop() override
        {
            // redraw preview with current color
            if (contentRef && contentRef->isMounted())
            {
                auto &c = canvas();
                uint16_t rgb565 = c.color565(r, g, b);
                c.fillRect(previewX, previewY, previewW, previewH, rgb565);
                c.drawRect(previewX, previewY, previewW, previewH, Theme::ButtonShadow);
            }
        }

    private:
        Container *contentRef{nullptr};
        int contentW{0}, contentH{0};
        uint8_t r{0}, g{0}, b{0};
        int previewX{0}, previewY{0}, previewW{0}, previewH{0};

        void applyColor()
        {
            setRgbLedColor(r, g, b);
        }

        void addColorRow(Container &cont, int cx, int ry, int w,
                         const char *label, uint8_t &val, uint16_t color)
        {
            auto lbl = std::make_unique<Label>(label, cx + 4, ry, 16, 22);
            lbl->setTextColor(Theme::TextColor, Theme::WindowBg);
            lbl->setTextSize(1);
            cont.addChild(std::move(lbl));

            // value label
            auto valLbl = std::make_unique<Label>(String(val), cx + 22, ry, 30, 22);
            valLbl->setTextColor(Theme::TextColor, Theme::WindowBg);
            valLbl->setTextSize(1);
            Label *valPtr = valLbl.get();
            cont.addChild(std::move(valLbl));

            // minus button
            auto minusBtn = std::make_unique<Button>("-", cx + 54, ry, 28, 22);
            minusBtn->setBackgroundColor(Theme::ButtonFace);
            minusBtn->setTextColor(Theme::TextColor, Theme::ButtonFace);
            minusBtn->setCallback([this, &val, valPtr]()
                                  {
                if (val >= 25) val -= 25; else val = 0;
                valPtr->setText(String(val));
                applyColor(); });
            cont.addChild(std::move(minusBtn));

            // plus button
            auto plusBtn = std::make_unique<Button>("+", cx + 86, ry, 28, 22);
            plusBtn->setBackgroundColor(Theme::ButtonFace);
            plusBtn->setTextColor(Theme::TextColor, Theme::ButtonFace);
            plusBtn->setCallback([this, &val, valPtr]()
                                {
                if (val <= 230) val += 25; else val = 255;
                valPtr->setText(String(val));
                applyColor(); });
            cont.addChild(std::move(plusBtn));

            // max button
            auto maxBtn = std::make_unique<Button>("Max", cx + 118, ry, 40, 22);
            maxBtn->setBackgroundColor(Theme::ButtonFace);
            maxBtn->setTextColor(Theme::TextColor, Theme::ButtonFace);
            maxBtn->setCallback([this, &val, valPtr]()
                                {
                val = 255;
                valPtr->setText(String(val));
                applyColor(); });
            cont.addChild(std::move(maxBtn));
        }
    };

} // namespace UI
