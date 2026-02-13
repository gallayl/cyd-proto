#pragma once

#include "../App.h"
#include "../elements/container.h"
#include "../elements/scrollable.h"
#include "../elements/label.h"
#include "../elements/button.h"
#include "../Theme.h"
#include "../../../../hw/WiFi.h"
#include <WiFi.h>
#include <memory>
#include <vector>

namespace UI
{

    class WifiApp : public App
    {
    public:
        const char *name() const override { return "WiFi"; }

        void setup(Container &cont, int w, int h) override
        {
            int cx, cy, cw, ch;
            cont.getBounds(cx, cy, cw, ch);

            auto scroll = std::make_unique<ScrollableContainer>();
            scroll->setBounds(cx, cy, cw, ch);
            scrollRef = scroll.get();
            scrollAreaX = cx;
            scrollAreaW = cw;

            int rowH = 14;
            int curY = cy + 4;

            addRow(*scroll, cx, curY, cw, "-- Connection --");
            curY += rowH;

            statusLabel = addRowLabel(*scroll, cx, curY, cw, "Status: ...");
            curY += rowH;
            ipLabel = addRowLabel(*scroll, cx, curY, cw, "IP: ...");
            curY += rowH;
            ssidLabel = addRowLabel(*scroll, cx, curY, cw, "SSID: ...");
            curY += rowH;
            rssiLabel = addRowLabel(*scroll, cx, curY, cw, "RSSI: ...");
            curY += rowH + 4;

            // Scan button
            auto scanBtn = std::make_unique<Button>("Scan", cx + 4, curY, 60, 22);
            scanBtn->setBackgroundColor(Theme::ButtonFace);
            scanBtn->setTextColor(Theme::TextColor, Theme::ButtonFace);
            scanBtn->setCallback([this]()
                                 { doScan(); });
            scroll->addChild(std::move(scanBtn));
            curY += 28;

            scanStartY = curY;
            scroll->setContentHeight(curY - cy);
            cont.addChild(std::move(scroll));
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
        ScrollableContainer *scrollRef{nullptr};
        int scrollAreaX{0}, scrollAreaW{0};
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
            if (!scrollRef)
                return;

            int cx = scrollAreaX;
            int w = scrollAreaW;
            int rowH = 14;
            int curY = scanStartY;

            int n = WiFi.scanNetworks();

            for (auto *lbl : scanLabels)
            {
                lbl->setText("");
            }
            scanLabels.clear();

            if (n <= 0)
            {
                scanLabels.push_back(addRowLabel(*scrollRef, cx, curY, w, "No networks found"));
                curY += rowH;
            }
            else
            {
                addRow(*scrollRef, cx, curY, w, "-- Networks --");
                curY += rowH;

                for (int i = 0; i < n && i < 10; i++)
                {
                    String line = WiFi.SSID(i) + " (" + String(WiFi.RSSI(i)) + ")";
                    scanLabels.push_back(addRowLabel(*scrollRef, cx, curY, w, line));
                    curY += rowH;
                }
            }

            int sx, sy, sw, sh;
            scrollRef->getBounds(sx, sy, sw, sh);
            scrollRef->setContentHeight(curY - sy);
        }

        void addRow(ScrollableContainer &sc, int cx, int ry, int w, const String &text)
        {
            auto lbl = std::make_unique<Label>(text, cx + 4, ry, w - 8, 12);
            lbl->setTextColor(Theme::TextColor, Theme::WindowBg);
            lbl->setTextSize(1);
            lbl->setAlign(TextAlign::LEFT);
            sc.addChild(std::move(lbl));
        }

        Label *addRowLabel(ScrollableContainer &sc, int cx, int ry, int w, const String &text)
        {
            auto lbl = std::make_unique<Label>(text, cx + 4, ry, w - 8, 12);
            lbl->setTextColor(Theme::TextColor, Theme::WindowBg);
            lbl->setTextSize(1);
            lbl->setAlign(TextAlign::LEFT);
            Label *ptr = lbl.get();
            sc.addChild(std::move(lbl));
            return ptr;
        }
    };

} // namespace UI
