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
            int baseY = 0;
            int cx, cy, cw, ch;
            cont.getBounds(cx, cy, cw, ch);

            // color preview element at top
            auto preview = std::make_unique<PreviewElement>(this);
            preview->setBounds(cx + 4, cy + 4, w - 8, 30);
            cont.addChild(std::move(preview));
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

    private:
        uint8_t r{0}, g{0}, b{0};

        class PreviewElement : public Element
        {
        public:
            explicit PreviewElement(RgbLedApp *a) : app(a) {}

            void draw() override
            {
                if (!mounted || !app)
                    return;
                auto &c = canvas();
                uint16_t rgb565 = c.color565(app->r, app->g, app->b);
                c.fillRect(x, y, width, height, rgb565);
                c.drawRect(x, y, width, height, Theme::ButtonShadow);
            }

        private:
            RgbLedApp *app;
        };

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
