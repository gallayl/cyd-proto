#pragma once

#include "../App.h"
#include "../elements/container.h"
#include "../elements/scrollable.h"
#include "../elements/label.h"
#include "../elements/button.h"
#include "../Theme.h"
#include "../Renderer.h"
#include "../../../registeredFeatures.h"
#include "../../../FeatureRegistry.h"
#include <ArduinoJson.h>
#include <Esp.h>
#include <memory>

namespace UI
{

    // Win95-style sunken progress bar for memory usage
    class MemoryBar : public Element
    {
    public:
        MemoryBar(int ix, int iy, int iw, int ih)
        {
            setBounds(ix, iy, iw, ih);
            refresh();
        }

        void refresh()
        {
            totalHeap = ESP.getHeapSize();
            freeHeap = ESP.getFreeHeap();
        }

        void draw() override
        {
            if (!mounted)
                return;
            auto &c = canvas();

            uint32_t usedHeap = totalHeap - freeHeap;
            float ratio = (totalHeap > 0) ? (float)usedHeap / totalHeap : 0;

            // sunken border (Win95 inset look)
            c.drawFastHLine(x, y, width, Theme::ButtonShadow);
            c.drawFastVLine(x, y, height, Theme::ButtonShadow);
            c.drawFastHLine(x, y + height - 1, width, Theme::ButtonHighlight);
            c.drawFastVLine(x + width - 1, y, height, Theme::ButtonHighlight);

            // inner area
            int innerX = x + 1;
            int innerY = y + 1;
            int innerW = width - 2;
            int innerH = height - 2;

            // background (white like Win95 progress bars)
            c.fillRect(innerX, innerY, innerW, innerH, TFT_WHITE);

            // filled portion (navy blue)
            int filledW = (int)(innerW * ratio);
            if (filledW > 0)
            {
                c.fillRect(innerX, innerY, filledW, innerH, Theme::TitleBarActive);
            }

            // text overlay
            uint32_t usedKB = usedHeap / 1024;
            uint32_t totalKB = totalHeap / 1024;
            String text = String(usedKB) + "KB / " + String(totalKB) + "KB";

            c.setTextSize(1);
            int16_t tw = c.textWidth(text);
            int16_t th = c.fontHeight();
            int16_t tx = innerX + (innerW - tw) / 2;
            int16_t ty = innerY + (innerH - th) / 2;

            // draw text with contrasting color depending on position
            c.setTextColor(TFT_BLACK, TFT_WHITE);
            c.setCursor(tx, ty);
            c.print(text);
        }

    private:
        uint32_t totalHeap{0};
        uint32_t freeHeap{0};
    };

    class FeaturesApp : public App
    {
    public:
        const char *name() const override { return "Features"; }

        void setup(Container &cont, int w, int h) override
        {
            parentContainer = &cont;
            buildUI();

            scheduleTimer(2000, [this]()
                          {
                if (memBar) {
                    memBar->refresh();
                    markDirty();
                } });
        }

    private:
        Container *parentContainer{nullptr};
        ScrollableContainer *scrollRef{nullptr};
        MemoryBar *memBar{nullptr};

        static String stateToString(FeatureState state)
        {
            switch (state)
            {
            case FeatureState::PENDING:
                return "PENDING";
            case FeatureState::SETUP:
                return "SETUP";
            case FeatureState::RUNNING:
                return "RUNNING";
            case FeatureState::ERROR:
                return "ERROR";
            case FeatureState::STOPPED:
                return "STOPPED";
            default:
                return "UNKNOWN";
            }
        }

        void buildUI()
        {
            if (!parentContainer)
                return;

            parentContainer->clear();

            int cx, cy, cw, ch;
            parentContainer->getBounds(cx, cy, cw, ch);

            auto scroll = std::make_unique<ScrollableContainer>();
            scroll->setBounds(cx, cy, cw, ch);
            scrollRef = scroll.get();

            int rowH = 14;
            int btnH = 18;
            int barH = 16;
            int curY = cy + 4;

            // memory usage bar
            addRow(*scroll, cx, curY, cw, "Heap:");
            curY += rowH;

            auto bar = std::make_unique<MemoryBar>(cx + 4, curY, cw - 8, barH);
            memBar = bar.get();
            scroll->addChild(std::move(bar));
            curY += barH + 6;

            addRow(*scroll, cx, curY, cw, "-- Registered Features --");
            curY += rowH;

            uint8_t count = FeatureRegistryInstance->GetFeatureCount();
            for (uint8_t i = 0; i < count; i++)
            {
                Feature *f = FeatureRegistryInstance->RegisteredFeatures[i];
                FeatureState state = f->GetFeatureState();
                String line = f->GetFeatureName() + ": " + stateToString(state);

                addRow(*scroll, cx, curY, cw - 50, line);

                bool canStart = (state == FeatureState::PENDING ||
                                 state == FeatureState::STOPPED ||
                                 state == FeatureState::ERROR);
                bool canStop = (state == FeatureState::RUNNING && f->hasTeardown());

                if (canStart || canStop)
                {
                    String featureName = f->GetFeatureName();
                    int btnW = 42;
                    int btnX = cx + cw - btnW - 4;

                    auto btn = std::make_unique<Button>(
                        canStop ? "Stop" : "Start",
                        btnX, curY, btnW, btnH);
                    btn->setBackgroundColor(Theme::ButtonFace);
                    btn->setTextColor(Theme::TextColor, Theme::ButtonFace);

                    if (canStop)
                    {
                        btn->setCallback([this, featureName]()
                                         {
                            FeatureRegistryInstance->TeardownFeature(featureName);
                            buildUI();
                            markDirty(); });
                    }
                    else
                    {
                        btn->setCallback([this, featureName]()
                                         {
                            FeatureRegistryInstance->SetupFeature(featureName);
                            buildUI();
                            markDirty(); });
                    }
                    scroll->addChild(std::move(btn));
                }

                curY += (canStart || canStop) ? btnH + 2 : rowH;
            }

            scroll->setContentHeight(curY - cy);
            parentContainer->addChild(std::move(scroll));
        }

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
