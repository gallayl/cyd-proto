#pragma once

#include "../App.h"
#include "../elements/container.h"
#include "../elements/scrollable.h"
#include "../elements/label.h"
#include "../elements/button.h"
#include "../Theme.h"
#include "../../Logging.h"
#include <ArduinoJson.h>
#include <memory>
#include <vector>

namespace UI
{

    class LogViewerApp : public App
    {
    public:
        const char *name() const override { return "Log Viewer"; }

        void setup(Container &cont, int w, int h) override
        {
            int cx, cy, cw, ch;
            cont.getBounds(cx, cy, cw, ch);

            auto scroll = std::make_unique<ScrollableContainer>();
            scroll->setBounds(cx, cy, cw, ch);
            scrollRef = scroll.get();
            scrollAreaX = cx;
            scrollAreaY = cy;
            scrollAreaW = cw;
            scrollAreaH = ch;

            populateLog();

            cont.addChild(std::move(scroll));

            scheduleTimer(3000, [this]()
                          { refreshLog(); });
        }

    private:
        ScrollableContainer *scrollRef{nullptr};
        int scrollAreaX{0}, scrollAreaY{0}, scrollAreaW{0}, scrollAreaH{0};
        std::vector<Label *> logLabels;
        int lastEntryCount{0};

        void populateLog()
        {
            if (!scrollRef)
                return;

            int rowH = 12;
            int curY = scrollAreaY + 2;

            const JsonDocument &entries = LoggerInstance->getEntries();
            JsonArrayConst arr = entries.as<JsonArrayConst>();
            int count = arr.size();

            // show last entries (newest at top for readability)
            int shown = 0;
            for (int i = count - 1; i >= 0 && shown < 50; i--, shown++)
            {
                JsonObjectConst entry = arr[i];
                const char *sev = entry["severity"] | "?";
                const char *msg = entry["message"] | "";
                String line = String("[") + sev + "] " + msg;

                auto lbl = std::make_unique<Label>(line, scrollAreaX + 2, curY, scrollAreaW - 4, 10);
                lbl->setTextColor(Theme::TextColor, Theme::WindowBg);
                lbl->setTextSize(1);
                lbl->setAlign(TextAlign::LEFT);
                logLabels.push_back(lbl.get());
                scrollRef->addChild(std::move(lbl));
                curY += rowH;
            }

            lastEntryCount = count;
            scrollRef->setContentHeight(curY - scrollAreaY);
        }

        void refreshLog()
        {
            if (!scrollRef)
                return;

            const JsonDocument &entries = LoggerInstance->getEntries();
            JsonArrayConst arr = entries.as<JsonArrayConst>();
            int count = arr.size();

            if (count == lastEntryCount)
                return;

            // remove old labels from the container so they are freed
            logLabels.clear();
            scrollRef->getContent().clear();

            int rowH = 12;
            int curY = scrollAreaY + 2;
            int shown = 0;
            for (int i = count - 1; i >= 0 && shown < 50; i--, shown++)
            {
                JsonObjectConst entry = arr[i];
                const char *sev = entry["severity"] | "?";
                const char *msg = entry["message"] | "";
                String line = String("[") + sev + "] " + msg;

                auto lbl = std::make_unique<Label>(line, scrollAreaX + 2, curY, scrollAreaW - 4, 10);
                lbl->setTextColor(Theme::TextColor, Theme::WindowBg);
                lbl->setTextSize(1);
                lbl->setAlign(TextAlign::LEFT);
                logLabels.push_back(lbl.get());
                scrollRef->addChild(std::move(lbl));
                curY += rowH;
            }

            lastEntryCount = count;
            scrollRef->setContentHeight(curY - scrollAreaY);
        }
    };

} // namespace UI
