#pragma once

#include "../App.h"
#include "../elements/container.h"
#include "../elements/label.h"
#include "../elements/button.h"
#include "../elements/groupbox.h"
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

            // Brightness group box
            auto brightnessGroup = std::make_unique<GroupBox>("Brightness", cx + 2, baseY, cw - 4, 56);
            int gx = brightnessGroup->contentX();
            int gy = brightnessGroup->contentY();

            // value label
            auto valLbl = std::make_unique<Label>(String(brightness), gx + 2, gy + 2, 40, 22);
            valLbl->setTextColor(Theme::TextColor, Theme::WindowBg);
            valLbl->setTextSize(1);
            brightnessLabel = valLbl.get();
            brightnessGroup->addChild(std::move(valLbl));

            // minus button
            auto minusBtn = std::make_unique<Button>("-", gx + 46, gy + 2, 36, 22);
            minusBtn->setBackgroundColor(Theme::ButtonFace);
            minusBtn->setTextColor(Theme::TextColor, Theme::ButtonFace);
            minusBtn->setCallback([this]()
                                  {
                if (brightness >= 25) brightness -= 25; else brightness = 0;
                applyBrightness(); });
            brightnessGroup->addChild(std::move(minusBtn));

            // plus button
            auto plusBtn = std::make_unique<Button>("+", gx + 86, gy + 2, 36, 22);
            plusBtn->setBackgroundColor(Theme::ButtonFace);
            plusBtn->setTextColor(Theme::TextColor, Theme::ButtonFace);
            plusBtn->setCallback([this]()
                                {
                if (brightness <= 230) brightness += 25; else brightness = 255;
                applyBrightness(); });
            brightnessGroup->addChild(std::move(plusBtn));

            // max button
            auto maxBtn = std::make_unique<Button>("Max", gx + 126, gy + 2, 40, 22);
            maxBtn->setBackgroundColor(Theme::ButtonFace);
            maxBtn->setTextColor(Theme::TextColor, Theme::ButtonFace);
            maxBtn->setCallback([this]()
                                {
                brightness = 255;
                applyBrightness(); });
            brightnessGroup->addChild(std::move(maxBtn));

            cont.addChild(std::move(brightnessGroup));
            baseY += 62;

            // Calibration group box
            auto calGroup = std::make_unique<GroupBox>("Touch Calibration", cx + 2, baseY, cw - 4, 48);
            int cgx = calGroup->contentX();
            int cgy = calGroup->contentY();

            auto calBtn = std::make_unique<Button>("Calibrate", cgx + 2, cgy + 2, 80, 22);
            calBtn->setBackgroundColor(Theme::ButtonFace);
            calBtn->setTextColor(Theme::TextColor, Theme::ButtonFace);
            calBtn->setCallback([]()
                                { calibrateScreen(); });
            calGroup->addChild(std::move(calBtn));

            cont.addChild(std::move(calGroup));
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
