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
#include "../../../../fs/VirtualFS.h"
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <memory>

#if ENABLE_SD_CARD
#include <SD.h>
#include "../../../Features/SdCard/SdCardFeature.h"
#endif

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
                String info = getStorageInfo();
                auto infoLbl = std::make_unique<Label>(info, btnX + btnW + 4, toolY, cw - (btnX + btnW + 4 - cx), toolbarH);
                infoLbl->setTextColor(Theme::TextColor, Theme::WindowBg);
                infoLbl->setTextSize(1);
                infoLbl->setAlign(TextAlign::RIGHT);
                storageLabel = infoLbl.get();
                cont.addChild(std::move(infoLbl));
            }

            // file list view (takes remaining space)
            int listY = toolY + toolbarH + 2;
            int listH = ch - (listY - cy);

            auto fl = std::make_unique<FileListView>(cx, listY, cw, listH);
            fl->setViewMode(FileListViewMode::List);
            fl->setOnItemActivated([this](int idx, const FileItem &item)
                                   {
                // Handle virtual root entries
                if (isVirtualRoot() && item.isDir)
                {
                    if (item.name == "Flash")
                        currentPath = "/flash/";
                    else if (item.name == "SD Card")
                        currentPath = "/sd/";
                    refreshFileList();
                    return;
                }

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
        Label *storageLabel{nullptr};
        int contentX{0}, contentY{0}, contentW{0}, contentH{0};

        bool isVirtualRoot() const
        {
            return currentPath == "/";
        }

        String getStorageInfo()
        {
            if (isVirtualRoot())
                return "";

            ResolvedPath resolved = resolveVirtualPath(currentPath);
            if (!resolved.valid || !resolved.fs)
                return "";

            if (resolved.fs == &LittleFS)
            {
                size_t totalBytes = LittleFS.totalBytes();
                size_t usedBytes = LittleFS.usedBytes();
                return String(usedBytes / 1024) + "K/" + String(totalBytes / 1024) + "K";
            }

#if ENABLE_SD_CARD
            if (resolved.fs == &SD && isSdCardMounted())
            {
                uint64_t totalBytes = SD.totalBytes();
                uint64_t usedBytes = SD.usedBytes();
                return String((uint32_t)(usedBytes / (1024 * 1024))) + "M/" + String((uint32_t)(totalBytes / (1024 * 1024))) + "M";
            }
#endif

            return "";
        }

        void showVirtualRoot()
        {
            if (!fileList)
                return;

            std::vector<FileItem> items;

            FileItem flashItem;
            flashItem.name = "Flash";
            flashItem.isDir = true;
            flashItem.size = LittleFS.usedBytes();
            items.push_back(std::move(flashItem));

#if ENABLE_SD_CARD
            FileItem sdItem;
            sdItem.name = "SD Card";
            sdItem.isDir = true;
            sdItem.size = isSdCardMounted() ? (uint32_t)(SD.usedBytes() / 1024) : 0;
            items.push_back(std::move(sdItem));
#endif

            fileList->setItems(items);

            if (pathLabel)
                pathLabel->setText("/");

            if (storageLabel)
                storageLabel->setText("");

            markDirty();
        }

        void refreshFileList()
        {
            if (!fileList)
                return;

            if (isVirtualRoot())
            {
                showVirtualRoot();
                return;
            }

            ResolvedPath resolved = resolveVirtualPath(currentPath);
            if (!resolved.valid || !resolved.fs)
            {
                showVirtualRoot();
                currentPath = "/";
                return;
            }

#if ENABLE_SD_CARD
            if (resolved.fs == &SD && !isSdCardMounted())
            {
                std::vector<FileItem> items;
                FileItem errItem;
                errItem.name = "(not mounted)";
                errItem.isDir = false;
                items.push_back(std::move(errItem));
                fileList->setItems(items);

                if (pathLabel)
                    pathLabel->setText(currentPath);
                if (storageLabel)
                    storageLabel->setText("");

                markDirty();
                return;
            }
#endif

            JsonDocument files = getFileList(*resolved.fs, resolved.localPath.c_str());
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

            if (storageLabel)
                storageLabel->setText(getStorageInfo());

            markDirty();
        }

        void navigateUp(Container &cont, int w, int h)
        {
            if (isVirtualRoot())
                return;

            // If at a filesystem root (e.g. "/flash" or "/sd"), go to virtual root
            if (currentPath == "/flash" || currentPath == "/flash/" ||
                currentPath == "/sd" || currentPath == "/sd/")
            {
                currentPath = "/";
                refreshFileList();
                return;
            }

            int lastSlash = currentPath.lastIndexOf('/');
            if (lastSlash <= 0)
            {
                currentPath = "/";
            }
            else
            {
                currentPath = currentPath.substring(0, lastSlash);
                // Don't go above the filesystem prefix
                if (currentPath == "/flash" || currentPath == "/sd")
                    currentPath += "/";
            }

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
