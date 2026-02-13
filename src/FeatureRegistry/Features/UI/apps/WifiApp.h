#pragma once

#include "../App.h"
#include "../elements/container.h"
#include "../elements/label.h"
#include "../elements/button.h"
#include "../Theme.h"
#include "../../../../hw/WiFi.h"
#include <WiFi.h>

namespace UI
{

    class WifiApp : public App
    {
    public:
        const char *name() const override { return "WiFi"; }

        void setup(Container &cont, int w, int h) override
        {
            contentRef = &cont;
            contentW = w;
            contentH = h;

            int cx, cy, cw, ch;
            cont.getBounds(cx, cy, cw, ch);
            int rowH = 14;
            int curY = cy + 4;

            addRow(cont, cx, curY, w, "-- Connection --");
            curY += rowH;

            statusLabel = addRowLabel(cont, cx, curY, w, "Status: ...");
            curY += rowH;
            ipLabel = addRowLabel(cont, cx, curY, w, "IP: ...");
            curY += rowH;
            ssidLabel = addRowLabel(cont, cx, curY, w, "SSID: ...");
            curY += rowH;
            rssiLabel = addRowLabel(cont, cx, curY, w, "RSSI: ...");
            curY += rowH + 4;

            // Scan button
            auto scanBtn = std::make_unique<Button>("Scan", cx + 4, curY, 60, 22);
            scanBtn->setBackgroundColor(Theme::ButtonFace);
            scanBtn->setTextColor(Theme::TextColor, Theme::ButtonFace);
            scanBtn->setCallback([this]()
                                 { doScan(); });
            cont.addChild(std::move(scanBtn));
            curY += 28;

            scanStartY = curY;
            refreshInfo();
        }

        void loop() override
        {
            unsigned long now = millis();
            if (now - lastRefresh > 5000)
            {
                refreshInfo();
                lastRefresh = now;
            }
        }

    private:
        Container *contentRef{nullptr};
        int contentW{0}, contentH{0};
        Label *statusLabel{nullptr};
        Label *ipLabel{nullptr};
        Label *ssidLabel{nullptr};
        Label *rssiLabel{nullptr};
        int scanStartY{0};
        unsigned long lastRefresh{0};
        std::vector<Label *> scanLabels;

        void refreshInfo()
        {
            wl_status_t st = WiFi.status();
            String statusStr;
            switch (st)
            {
            case WL_CONNECTED:
                statusStr = "Connected";
                break;
            case WL_IDLE_STATUS:
                statusStr = "Idle";
                break;
            case WL_NO_SSID_AVAIL:
                statusStr = "No SSID";
                break;
            case WL_DISCONNECTED:
                statusStr = "Disconnected";
                break;
            default:
                statusStr = "Other (" + String((int)st) + ")";
                break;
            }
            if (statusLabel)
                statusLabel->setText("Status: " + statusStr);
            if (ipLabel)
                ipLabel->setText("IP: " + WiFi.localIP().toString());
            if (ssidLabel)
                ssidLabel->setText("SSID: " + WiFi.SSID());
            if (rssiLabel)
            {
                int32_t rssi = WiFi.RSSI();
                rssiLabel->setText("RSSI: " + String(rssi) + " dBm (" + getSignalStrength(rssi) + ")");
            }
        }

        void doScan()
        {
            if (!contentRef)
                return;

            int cx, cy, cw, ch;
            contentRef->getBounds(cx, cy, cw, ch);
            int w = contentW;
            int rowH = 14;
            int curY = scanStartY;

            int n = WiFi.scanNetworks();

            // clear previous scan labels from the screen by overwriting text
            for (auto *lbl : scanLabels)
            {
                lbl->setText("");
            }
            scanLabels.clear();

            if (n <= 0)
            {
                scanLabels.push_back(addRowLabel(*contentRef, cx, curY, w, "No networks found"));
                return;
            }

            addRow(*contentRef, cx, curY, w, "-- Networks --");
            curY += rowH;

            for (int i = 0; i < n && i < 10; i++)
            {
                String line = WiFi.SSID(i) + " (" + String(WiFi.RSSI(i)) + ")";
                scanLabels.push_back(addRowLabel(*contentRef, cx, curY, w, line));
                curY += rowH;
            }
        }

        void addRow(Container &cont, int cx, int ry, int w, const String &text)
        {
            auto lbl = std::make_unique<Label>(text, cx + 4, ry, w - 8, 12);
            lbl->setTextColor(Theme::TextColor, Theme::WindowBg);
            lbl->setTextSize(1);
            lbl->setAlign(TextAlign::LEFT);
            cont.addChild(std::move(lbl));
        }

        Label *addRowLabel(Container &cont, int cx, int ry, int w, const String &text)
        {
            auto lbl = std::make_unique<Label>(text, cx + 4, ry, w - 8, 12);
            lbl->setTextColor(Theme::TextColor, Theme::WindowBg);
            lbl->setTextSize(1);
            lbl->setAlign(TextAlign::LEFT);
            Label *ptr = lbl.get();
            cont.addChild(std::move(lbl));
            return ptr;
        }
    };

} // namespace UI
