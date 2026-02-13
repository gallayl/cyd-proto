#pragma once

#include "../App.h"
#include "../elements/container.h"
#include "../elements/label.h"
#include "../elements/scrollable.h"
#include "../Theme.h"
#include "../../infoCustomCommand.h"
#include <ArduinoJson.h>
#include <Arduino.h>

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

            int rowH = 14;
            int curY = cy + 4;

            JsonDocument info = getInfo();

            // ESP section
            addRow(cont, cx, curY, w, "-- ESP --");
            curY += rowH;

            if (info.containsKey("esp"))
            {
                JsonObject esp = info["esp"];
                addRow(cont, cx, curY, w, "SDK: " + String(esp["sdkVersion"].as<const char *>()));
                curY += rowH;
                addRow(cont, cx, curY, w, "CPU: " + String(esp["cpuFreqMhz"].as<int>()) + " MHz");
                curY += rowH;
                addRow(cont, cx, curY, w, "Heap: " + String(esp["freeHeap"].as<uint32_t>()) + " B");
                curY += rowH;
                addRow(cont, cx, curY, w, "Sketch: " + String(esp["freeSkSpace"].as<uint32_t>()) + " B");
                curY += rowH;
            }

            curY += 4;
            addRow(cont, cx, curY, w, "-- Flash --");
            curY += rowH;

            if (info.containsKey("flash"))
            {
                JsonObject flash = info["flash"];
                addRow(cont, cx, curY, w, "Size: " + String(flash["size"].as<uint32_t>()) + " B");
                curY += rowH;
                addRow(cont, cx, curY, w, "Speed: " + String(flash["speed"].as<uint32_t>() / 1000000) + " MHz");
                curY += rowH;
            }

            curY += 4;
            addRow(cont, cx, curY, w, "-- Filesystem --");
            curY += rowH;

            if (info.containsKey("fs"))
            {
                JsonObject fs = info["fs"];
                addRow(cont, cx, curY, w, "Total: " + String(fs["totalBytes"].as<uint32_t>()) + " B");
                curY += rowH;
                addRow(cont, cx, curY, w, "Used: " + String(fs["usedBytes"].as<uint32_t>()) + " B");
                curY += rowH;
            }

            curY += 4;
            addRow(cont, cx, curY, w, "-- WiFi --");
            curY += rowH;
            addRow(cont, cx, curY, w, "IP: " + WiFi.localIP().toString());
            curY += rowH;
            addRow(cont, cx, curY, w, "MAC: " + WiFi.macAddress());
            curY += rowH;
            addRow(cont, cx, curY, w, "RSSI: " + String(WiFi.RSSI()) + " dBm");
            curY += rowH;
        }

    private:
        void addRow(Container &cont, int cx, int ry, int w, const String &text)
        {
            auto lbl = std::make_unique<Label>(text, cx + 4, ry, w - 8, 12);
            lbl->setTextColor(Theme::TextColor, Theme::WindowBg);
            lbl->setTextSize(1);
            lbl->setAlign(TextAlign::LEFT);
            cont.addChild(std::move(lbl));
        }
    };

} // namespace UI
