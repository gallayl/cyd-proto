#pragma once

#include "../App.h"
#include "../elements/container.h"
#include "../elements/label.h"
#include "../elements/scrollable.h"
#include "../Theme.h"
#include "../../infoCustomCommand.h"
#include <ArduinoJson.h>
#include <Arduino.h>
#include <memory>

namespace UI
{

    class InfoApp : public App
    {
    public:
        const char *name() const override { return "Info"; }

        void setup(Container &cont, int w, int h) override
        {
            int cx, cy, cw, ch;
            cont.getBounds(cx, cy, cw, ch);

            auto scroll = std::make_unique<ScrollableContainer>();
            scroll->setBounds(cx, cy, cw, ch);

            int rowH = 14;
            int curY = cy + 4;

            JsonDocument info = getInfo();

            // ESP section
            addRow(*scroll, cx, curY, cw, "-- ESP --");
            curY += rowH;

            if (info.containsKey("esp"))
            {
                JsonObject esp = info["esp"];
                addRow(*scroll, cx, curY, cw, "SDK: " + String(esp["sdkVersion"].as<const char *>()));
                curY += rowH;
                addRow(*scroll, cx, curY, cw, "CPU: " + String(esp["cpuFreqMhz"].as<int>()) + " MHz");
                curY += rowH;
                addRow(*scroll, cx, curY, cw, "Heap: " + String(esp["freeHeap"].as<uint32_t>()) + " B");
                curY += rowH;
                addRow(*scroll, cx, curY, cw, "Sketch: " + String(esp["freeSkSpace"].as<uint32_t>()) + " B");
                curY += rowH;
            }

            curY += 4;
            addRow(*scroll, cx, curY, cw, "-- Flash --");
            curY += rowH;

            if (info.containsKey("flash"))
            {
                JsonObject flash = info["flash"];
                addRow(*scroll, cx, curY, cw, "Size: " + String(flash["size"].as<uint32_t>()) + " B");
                curY += rowH;
                addRow(*scroll, cx, curY, cw, "Speed: " + String(flash["speed"].as<uint32_t>() / 1000000) + " MHz");
                curY += rowH;
            }

            curY += 4;
            addRow(*scroll, cx, curY, cw, "-- Filesystem --");
            curY += rowH;

            if (info.containsKey("fs"))
            {
                JsonObject fs = info["fs"];
                addRow(*scroll, cx, curY, cw, "Total: " + String(fs["totalBytes"].as<uint32_t>()) + " B");
                curY += rowH;
                addRow(*scroll, cx, curY, cw, "Used: " + String(fs["usedBytes"].as<uint32_t>()) + " B");
                curY += rowH;
            }

            curY += 4;
            addRow(*scroll, cx, curY, cw, "-- WiFi --");
            curY += rowH;
            addRow(*scroll, cx, curY, cw, "IP: " + WiFi.localIP().toString());
            curY += rowH;
            addRow(*scroll, cx, curY, cw, "MAC: " + WiFi.macAddress());
            curY += rowH;
            addRow(*scroll, cx, curY, cw, "RSSI: " + String(WiFi.RSSI()) + " dBm");
            curY += rowH;

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
