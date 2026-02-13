#pragma once

#include "../App.h"
#include "../elements/container.h"
#include "../elements/label.h"
#include "../elements/button.h"
#include "../Theme.h"
#include "../../../../hw/lightSensor.h"
#include <Arduino.h>
#include <memory>

namespace UI
{

    class SensorsApp : public App
    {
    public:
        const char *name() const override { return "Sensors"; }

        void setup(Container &cont, int w, int h) override
        {
            int cx, cy, cw, ch;
            cont.getBounds(cx, cy, cw, ch);

            int rowH = 14;
            int curY = cy + 4;

            auto header = std::make_unique<Label>("-- Light Sensor --", cx + 4, curY, cw - 8, 12);
            header->setTextColor(Theme::TextColor, Theme::WindowBg);
            header->setTextSize(1);
            header->setAlign(TextAlign::LEFT);
            cont.addChild(std::move(header));
            curY += rowH;

            auto lbl = std::make_unique<Label>("Value: ...", cx + 4, curY, cw - 8, 12);
            lbl->setTextColor(Theme::TextColor, Theme::WindowBg);
            lbl->setTextSize(1);
            lbl->setAlign(TextAlign::LEFT);
            lightLabel = lbl.get();
            cont.addChild(std::move(lbl));
            curY += rowH + 8;

            auto header2 = std::make_unique<Label>("-- Hall Sensor --", cx + 4, curY, cw - 8, 12);
            header2->setTextColor(Theme::TextColor, Theme::WindowBg);
            header2->setTextSize(1);
            header2->setAlign(TextAlign::LEFT);
            cont.addChild(std::move(header2));
            curY += rowH;

            auto lbl2 = std::make_unique<Label>("Value: ...", cx + 4, curY, cw - 8, 12);
            lbl2->setTextColor(Theme::TextColor, Theme::WindowBg);
            lbl2->setTextSize(1);
            lbl2->setAlign(TextAlign::LEFT);
            hallLabel = lbl2.get();
            cont.addChild(std::move(lbl2));
            curY += rowH + 8;

            auto refreshBtn = std::make_unique<Button>("Refresh", cx + 4, curY, 70, 22);
            refreshBtn->setBackgroundColor(Theme::ButtonFace);
            refreshBtn->setTextColor(Theme::TextColor, Theme::ButtonFace);
            refreshBtn->setCallback([this]()
                                    { refreshValues(); });
            cont.addChild(std::move(refreshBtn));

            refreshValues();
        }

    private:
        Label *lightLabel{nullptr};
        Label *hallLabel{nullptr};

        void refreshValues()
        {
            if (lightLabel)
            {
                uint16_t val = readLightSensor();
                lightLabel->setText("Value: " + String(val));
            }
            if (hallLabel)
            {
                int val = hallRead();
                hallLabel->setText("Value: " + String(val));
            }
        }
    };

} // namespace UI
