#pragma once

#include "../App.h"
#include "../elements/container.h"
#include "../elements/scrollable.h"
#include "../elements/label.h"
#include "../Theme.h"
#include "../../../../api/list.h"
#include <ArduinoJson.h>
#include <memory>

namespace UI
{

    class FileManagerApp : public App
    {
    public:
        const char *name() const override { return "File Manager"; }

        void setup(Container &cont, int w, int h) override
        {
            int cx, cy, cw, ch;
            cont.getBounds(cx, cy, cw, ch);

            auto scroll = std::make_unique<ScrollableContainer>();
            scroll->setBounds(cx, cy, cw, ch);

            int rowH = 14;
            int curY = cy + 4;

            addRow(*scroll, cx, curY, cw, "-- Files --");
            curY += rowH;

            JsonDocument files = getFileList();
            JsonArray arr = files.as<JsonArray>();

            if (arr.size() == 0)
            {
                addRow(*scroll, cx, curY, cw, "(no files)");
                curY += rowH;
            }
            else
            {
                for (JsonObject file : arr)
                {
                    const char *name = file["name"] | "?";
                    uint32_t size = file["size"] | 0;
                    bool isDir = file["isDir"] | false;

                    String line;
                    if (isDir)
                        line = String("[") + name + "]";
                    else
                        line = String(name) + " (" + String(size) + "B)";

                    addRow(*scroll, cx, curY, cw, line);
                    curY += rowH;
                }
            }

            curY += 4;
            addRow(*scroll, cx, curY, cw, "-- Storage --");
            curY += rowH;

            size_t totalBytes = LittleFS.totalBytes();
            size_t usedBytes = LittleFS.usedBytes();
            addRow(*scroll, cx, curY, cw, "Total: " + String(totalBytes) + " B");
            curY += rowH;
            addRow(*scroll, cx, curY, cw, "Used: " + String(usedBytes) + " B");
            curY += rowH;
            addRow(*scroll, cx, curY, cw, "Free: " + String(totalBytes - usedBytes) + " B");
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
