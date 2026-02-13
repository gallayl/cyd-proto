#pragma once

#include "../App.h"
#include "../elements/container.h"
#include "../elements/scrollable.h"
#include "../elements/label.h"
#include "../Theme.h"
#include "../../../registeredFeatures.h"
#include <ArduinoJson.h>
#include <memory>

namespace UI
{

    class FeaturesApp : public App
    {
    public:
        const char *name() const override { return "Features"; }

        void setup(Container &cont, int w, int h) override
        {
            int cx, cy, cw, ch;
            cont.getBounds(cx, cy, cw, ch);

            auto scroll = std::make_unique<ScrollableContainer>();
            scroll->setBounds(cx, cy, cw, ch);

            int rowH = 14;
            int curY = cy + 4;

            addRow(*scroll, cx, curY, cw, "-- Registered Features --");
            curY += rowH;

            for (JsonPair kv : registeredFeatures.as<JsonObject>())
            {
                const char *featureName = kv.value()["name"] | kv.key().c_str();
                int state = kv.value()["state"] | 0;
                String stateStr;
                switch (state)
                {
                case 0:
                    stateStr = "PENDING";
                    break;
                case 1:
                    stateStr = "SETUP";
                    break;
                case 2:
                    stateStr = "RUNNING";
                    break;
                case 3:
                    stateStr = "ERROR";
                    break;
                default:
                    stateStr = "UNKNOWN";
                    break;
                }

                String line = String(featureName) + ": " + stateStr;
                addRow(*scroll, cx, curY, cw, line);
                curY += rowH;
            }

            scroll->setContentHeight(curY - cy);
            cont.addChild(std::move(scroll));
        }

    private:
        void addRow(ScrollableContainer &sc, int cx, int ry, int w, const String &text)
        {
            auto lbl = std::make_unique<Label>(text, cx + 4, ry, w - 8, 12);
            lbl->setTextColor(Theme::TextColor, Theme::WindowBg);
            lbl->setTextSize(1);
            lbl->setAlign(TextAlign::LEFT);
            sc.addChild(std::move(lbl));
        }
    };

} // namespace UI
