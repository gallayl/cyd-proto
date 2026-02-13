#pragma once

#include "../App.h"
#include "../elements/container.h"
#include "../elements/label.h"
#include "../elements/button.h"
#include "../Theme.h"
#include "../Renderer.h"
#include "../Calibration.h"
#include <memory>

extern LGFX tft;

namespace UI
{

    class DisplaySettingsApp : public App
    {
    public:
        const char *name() const override { return "Display"; }

        void setup(Container &cont, int w, int h) override
        {
            int cx, cy, cw, ch;
            cont.getBounds(cx, cy, cw, ch);

            int baseY = cy + 4;

            // Brightness section
            auto header = std::make_unique<Label>("Brightness", cx + 4, baseY, cw - 8, 14);
            header->setTextColor(Theme::TextColor, Theme::WindowBg);
            header->setTextSize(1);
            header->setAlign(TextAlign::LEFT);
            cont.addChild(std::move(header));
            baseY += 18;

            // value label
            auto valLbl = std::make_unique<Label>(String(brightness), cx + 4, baseY, 40, 22);
            valLbl->setTextColor(Theme::TextColor, Theme::WindowBg);
            valLbl->setTextSize(1);
            brightnessLabel = valLbl.get();
            cont.addChild(std::move(valLbl));

            // minus button
            auto minusBtn = std::make_unique<Button>("-", cx + 48, baseY, 36, 22);
            minusBtn->setBackgroundColor(Theme::ButtonFace);
            minusBtn->setTextColor(Theme::TextColor, Theme::ButtonFace);
            minusBtn->setCallback([this]()
                                  {
                if (brightness >= 25) brightness -= 25; else brightness = 0;
                applyBrightness(); });
            cont.addChild(std::move(minusBtn));

            // plus button
            auto plusBtn = std::make_unique<Button>("+", cx + 88, baseY, 36, 22);
            plusBtn->setBackgroundColor(Theme::ButtonFace);
            plusBtn->setTextColor(Theme::TextColor, Theme::ButtonFace);
            plusBtn->setCallback([this]()
                                {
                if (brightness <= 230) brightness += 25; else brightness = 255;
                applyBrightness(); });
            cont.addChild(std::move(plusBtn));

            // max button
            auto maxBtn = std::make_unique<Button>("Max", cx + 128, baseY, 40, 22);
            maxBtn->setBackgroundColor(Theme::ButtonFace);
            maxBtn->setTextColor(Theme::TextColor, Theme::ButtonFace);
            maxBtn->setCallback([this]()
                                {
                brightness = 255;
                applyBrightness(); });
            cont.addChild(std::move(maxBtn));

            baseY += 34;

            // Calibration section
            auto calHeader = std::make_unique<Label>("Touch Calibration", cx + 4, baseY, cw - 8, 14);
            calHeader->setTextColor(Theme::TextColor, Theme::WindowBg);
            calHeader->setTextSize(1);
            calHeader->setAlign(TextAlign::LEFT);
            cont.addChild(std::move(calHeader));
            baseY += 18;

            auto calBtn = std::make_unique<Button>("Calibrate", cx + 4, baseY, 80, 22);
            calBtn->setBackgroundColor(Theme::ButtonFace);
            calBtn->setTextColor(Theme::TextColor, Theme::ButtonFace);
            calBtn->setCallback([]()
                                { calibrateScreen(); });
            cont.addChild(std::move(calBtn));
        }

    private:
        uint8_t brightness{128};
        Label *brightnessLabel{nullptr};

        void applyBrightness()
        {
            tft.setBrightness(brightness);
            if (brightnessLabel)
                brightnessLabel->setText(String(brightness));
        }
    };

} // namespace UI
