#pragma once

#include "../App.h"
#include "../elements/container.h"
#include "../elements/scrollable.h"
#include "../elements/label.h"
#include "../elements/button.h"
#include "../elements/filelistview.h"
#include "../Theme.h"
#include "../WindowManager.h"
#include "../ActionQueue.h"
#include "../../../../api/list.h"
#include "../../../../ActionRegistry/ActionRegistry.h"
#include "../../../../ActionRegistry/FeatureAction.h"
#include <ArduinoJson.h>
#include <LittleFS.h>
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

            contentW = cw;
            contentH = ch;
            contentX = cx;
            contentY = cy;

            // toolbar row: view mode buttons + path label
            const int toolbarH = 18;
            int btnW = 36;
            int btnX = cx;

            // path label
            auto pathLbl = std::make_unique<Label>(currentPath, cx, cy, cw, toolbarH);
            pathLbl->setTextColor(Theme::TextColor, Theme::WindowBg);
            pathLbl->setTextSize(1);
            pathLbl->setAlign(TextAlign::LEFT);
            pathLabel = pathLbl.get();
            cont.addChild(std::move(pathLbl));

            // view mode buttons in toolbar (below path)
            int toolY = cy + toolbarH;

            auto backBtn = std::make_unique<Button>("<-", cx, toolY, btnW, toolbarH);
            backBtn->setBackgroundColor(Theme::ButtonFace);
            backBtn->setTextColor(Theme::TextColor, Theme::ButtonFace);
            backBtn->setTextSize(1);
            backBtn->setCallback([this, &cont, w, h]()
                                 { navigateUp(cont, w, h); });
            cont.addChild(std::move(backBtn));
            btnX = cx + btnW + 2;

            auto listBtn = std::make_unique<Button>("List", btnX, toolY, btnW, toolbarH);
            listBtn->setBackgroundColor(Theme::ButtonFace);
            listBtn->setTextColor(Theme::TextColor, Theme::ButtonFace);
            listBtn->setTextSize(1);
            listBtn->setCallback([this]()
                                 {
                if (fileList)
                {
                    fileList->setViewMode(FileListViewMode::List);
                    markDirty();
                } });
            cont.addChild(std::move(listBtn));
            btnX += btnW + 2;

            auto iconBtn = std::make_unique<Button>("Icon", btnX, toolY, btnW, toolbarH);
            iconBtn->setBackgroundColor(Theme::ButtonFace);
            iconBtn->setTextColor(Theme::TextColor, Theme::ButtonFace);
            iconBtn->setTextSize(1);
            iconBtn->setCallback([this]()
                                 {
                if (fileList)
                {
                    fileList->setViewMode(FileListViewMode::Icons);
                    markDirty();
                } });
            cont.addChild(std::move(iconBtn));
            btnX += btnW + 2;

            auto detBtn = std::make_unique<Button>("Det", btnX, toolY, btnW, toolbarH);
            detBtn->setBackgroundColor(Theme::ButtonFace);
            detBtn->setTextColor(Theme::TextColor, Theme::ButtonFace);
            detBtn->setTextSize(1);
            detBtn->setCallback([this]()
                                 {
                if (fileList)
                {
                    fileList->setViewMode(FileListViewMode::Details);
                    markDirty();
                } });
            cont.addChild(std::move(detBtn));

            // storage info label
            {
                size_t totalBytes = LittleFS.totalBytes();
                size_t usedBytes = LittleFS.usedBytes();
                String info = String(usedBytes / 1024) + "K/" + String(totalBytes / 1024) + "K";

                auto infoLbl = std::make_unique<Label>(info, btnX + btnW + 4, toolY, cw - (btnX + btnW + 4 - cx), toolbarH);
                infoLbl->setTextColor(Theme::TextColor, Theme::WindowBg);
                infoLbl->setTextSize(1);
                infoLbl->setAlign(TextAlign::RIGHT);
                cont.addChild(std::move(infoLbl));
            }

            // file list view (takes remaining space)
            int listY = toolY + toolbarH + 2;
            int listH = ch - (listY - cy);

            auto fl = std::make_unique<FileListView>(cx, listY, cw, listH);
            fl->setViewMode(FileListViewMode::List);
            fl->setOnItemActivated([this](int idx, const FileItem &item)
                                   {
                if (item.isDir)
                {
                    if (currentPath.endsWith("/"))
                        currentPath += item.name;
                    else
                        currentPath += "/" + item.name;
                    refreshFileList();
                    return;
                }
                // execute .be files
                if (item.name.endsWith(".be"))
                {
                    executeBerryFile(item.name);
                } });
            fileList = fl.get();
            cont.addChild(std::move(fl));

            refreshFileList();
        }

    private:
        String currentPath{"/"};
        FileListView *fileList{nullptr};
        Label *pathLabel{nullptr};
        int contentX{0}, contentY{0}, contentW{0}, contentH{0};

        void refreshFileList()
        {
            if (!fileList)
                return;

            JsonDocument files = getFileList(currentPath.c_str());
            JsonArray arr = files.as<JsonArray>();

            std::vector<FileItem> items;
            for (JsonObject file : arr)
            {
                FileItem item;
                item.name = String(file["name"] | "?");
                item.size = file["size"] | 0;
                item.isDir = file["isDir"] | false;
                item.lastWrite = file["lastWrite"] | 0;
                items.push_back(std::move(item));
            }

            fileList->setItems(items);

            if (pathLabel)
                pathLabel->setText(currentPath);

            markDirty();
        }

        void navigateUp(Container &cont, int w, int h)
        {
            if (currentPath == "/" || currentPath.isEmpty())
                return;

            int lastSlash = currentPath.lastIndexOf('/');
            if (lastSlash <= 0)
                currentPath = "/";
            else
                currentPath = currentPath.substring(0, lastSlash);

            refreshFileList();
        }

        void executeBerryFile(const String &filename)
        {
            String fullPath;
            if (currentPath.endsWith("/"))
                fullPath = currentPath + filename;
            else
                fullPath = currentPath + "/" + filename;

            String cmd = "berry run " + fullPath;
            UI::queueAction([cmd]()
                            { ActionRegistryInstance->Execute(cmd, Transport::SCRIPTING); });
        }
    };

} // namespace UI
