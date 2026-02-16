#include "../../../config.h"

#if ENABLE_UI

#include <string>
#include <cstdlib>
#include <Esp.h>
#include <esp_timer.h>
#include "../Logging.h"
#include "../../../ActionRegistry/ActionRegistry.h"

#include "ActionQueue.h"
#include "Renderer.h"
#include "Desktop.h"
#include "WindowManager.h"
#include "UITaskQueue.h"
#include <LovyanGFX.hpp>
#include "../../../hw/Screen.h"
#include "../../Feature.h"
#include "../../../ActionRegistry/FeatureAction.h"
#include "../../../CommandInterpreter/CommandParser.h"
#include "./Calibration.h"

static volatile bool frameReady = true;
static esp_timer_handle_t frameTimer = nullptr;
static bool uiTaskInitDone = false;

// --- Screen command handler ---

static std::string screenCommandHandlerImpl(const std::string &command)
{
    std::string sub = CommandParser::getCommandParameter(command, 1);

    if (sub == "calibrate")
    {
        calibrateScreen();
        delay(1000); // Let user see "Calibration complete!" message
        tft.fillScreen(TFT_BLACK);
        tft.waitDisplay(); // Wait for fillScreen to complete
        if (!UI::reinitRenderer())
        {
            loggerInstance->Error("Failed to reinit renderer after calibration");
            return std::string("{\"event\":\"calibrate\", \"status\":\"error\", \"message\":\"renderer_init_failed\"}");
        }
        UI::windowManager().relayoutAll();
        frameReady = true; // Ensure next frame will render
        UI::markDirty();   // Ensure UI knows it needs redraw
        loggerInstance->Info("Calibrated touch screen");
        return std::string("{\"event\":\"calibrate\", \"status\":\"success\"}");
    }
    else if (sub == "demo")
    {
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(0, 0);
        tft.setTextColor(TFT_GREENYELLOW);
        tft.setTextSize(2);
        tft.println("Hello World!");

        tft.drawCircle(120, 160, 50, TFT_MAGENTA);
        tft.drawEllipse(200, 160, 60, 40, TFT_GOLD);

        loggerInstance->Info("Displayed hello world demo");
        return std::string("{\"event\":\"helloDemo\"}");
    }
    else if (sub == "rotate")
    {
        std::string rotateParam = CommandParser::getCommandParameter(command, 2);
        uint16_t rotate = 0;
        if (rotateParam.length() > 0)
        {
            rotate = (uint16_t)strtoul(rotateParam.c_str(), NULL, 0);
        }
        tft.setRotation(rotate);
        tft.fillScreen(TFT_BLACK);
        tft.waitDisplay(); // Wait for fillScreen to complete
        if (!UI::reinitRenderer())
        {
            loggerInstance->Error("Failed to reinit renderer after rotation");
            return std::string("{\"event\":\"rotate\",\"rotation\":") + std::to_string(rotate) +
                   ",\"error\":\"renderer_init_failed\"}";
        }
        readCalibrationData(); // Loads rotation-specific calibration
        UI::windowManager().relayoutAll();
        frameReady = true; // Ensure next frame will render
        UI::markDirty();   // Ensure UI knows it needs redraw
        loggerInstance->Info("Screen rotated to " + std::to_string(rotate));
        return std::string("{\"event\":\"rotate\",\"rotation\":") + std::to_string(rotate) + "}";
    }
    else if (sub == "clear")
    {
        std::string colorParam = CommandParser::getCommandParameter(command, 2);
        uint16_t color = TFT_BLACK;
        if (colorParam.length() > 0)
        {
            color = (uint16_t)strtoul(colorParam.c_str(), NULL, 0);
        }
        tft.fillScreen(color);
        loggerInstance->Info("Screen cleared");
        return std::string("{\"event\":\"clear\",\"color\":\"") + colorParam + "\"}";
    }
    else if (sub == "text")
    {
        int x = atoi(CommandParser::getCommandParameter(command, 2).c_str());
        int y = atoi(CommandParser::getCommandParameter(command, 3).c_str());
        uint16_t fg = (uint16_t)strtoul(CommandParser::getCommandParameter(command, 4).c_str(), NULL, 0);
        uint16_t bg = (uint16_t)strtoul(CommandParser::getCommandParameter(command, 5).c_str(), NULL, 0);
        int sz = atoi(CommandParser::getCommandParameter(command, 6).c_str());
        int count = 0;
        int pos = -1;
        for (int i = 0; i < (int)command.length(); ++i)
        {
            if (command[i] == ' ')
            {
                if (++count == 6)
                {
                    pos = i;
                    break;
                }
            }
        }
        std::string msg = (pos >= 0) ? command.substr(pos + 1) : std::string();
        tft.setCursor(x, y);
        tft.setTextColor(fg, bg);
        tft.setTextSize(sz);
        tft.print(msg.c_str());
        loggerInstance->Info("Drew text: " + msg);
        return std::string("{\"event\":\"text\"}");
    }
    else if (sub == "pixel")
    {
        int x = atoi(CommandParser::getCommandParameter(command, 2).c_str());
        int y = atoi(CommandParser::getCommandParameter(command, 3).c_str());
        uint16_t color = (uint16_t)strtoul(CommandParser::getCommandParameter(command, 4).c_str(), NULL, 0);
        tft.drawPixel(x, y, color);
        loggerInstance->Info("Drew pixel at " + std::to_string(x) + "," + std::to_string(y));
        return std::string("{\"event\":\"pixel\"}");
    }
    else if (sub == "rect")
    {
        int x = atoi(CommandParser::getCommandParameter(command, 2).c_str());
        int y = atoi(CommandParser::getCommandParameter(command, 3).c_str());
        int w = atoi(CommandParser::getCommandParameter(command, 4).c_str());
        int h = atoi(CommandParser::getCommandParameter(command, 5).c_str());
        uint16_t color = (uint16_t)strtoul(CommandParser::getCommandParameter(command, 6).c_str(), NULL, 0);
        tft.drawRect(x, y, w, h, color);
        loggerInstance->Info("Drew rect at " + std::to_string(x) + "," + std::to_string(y));
        return std::string("{\"event\":\"rect\"}");
    }
    else if (sub == "fillrect")
    {
        int x = atoi(CommandParser::getCommandParameter(command, 2).c_str());
        int y = atoi(CommandParser::getCommandParameter(command, 3).c_str());
        int w = atoi(CommandParser::getCommandParameter(command, 4).c_str());
        int h = atoi(CommandParser::getCommandParameter(command, 5).c_str());
        uint16_t color = (uint16_t)strtoul(CommandParser::getCommandParameter(command, 6).c_str(), NULL, 0);
        tft.fillRect(x, y, w, h, color);
        loggerInstance->Info("Drew filled rect at " + std::to_string(x) + "," + std::to_string(y));
        return std::string("{\"event\":\"fillrect\"}");
    }
    else if (sub == "circle")
    {
        int x = atoi(CommandParser::getCommandParameter(command, 2).c_str());
        int y = atoi(CommandParser::getCommandParameter(command, 3).c_str());
        int r = atoi(CommandParser::getCommandParameter(command, 4).c_str());
        uint16_t color = (uint16_t)strtoul(CommandParser::getCommandParameter(command, 5).c_str(), NULL, 0);
        tft.drawCircle(x, y, r, color);
        loggerInstance->Info("Drew circle at " + std::to_string(x) + "," + std::to_string(y));
        return std::string("{\"event\":\"circle\"}");
    }
    else if (sub == "fillcircle")
    {
        int x = atoi(CommandParser::getCommandParameter(command, 2).c_str());
        int y = atoi(CommandParser::getCommandParameter(command, 3).c_str());
        int r = atoi(CommandParser::getCommandParameter(command, 4).c_str());
        uint16_t color = (uint16_t)strtoul(CommandParser::getCommandParameter(command, 5).c_str(), NULL, 0);
        tft.fillCircle(x, y, r, color);
        loggerInstance->Info("Drew filled circle at " + std::to_string(x) + "," + std::to_string(y));
        return std::string("{\"event\":\"fillcircle\"}");
    }
    else if (sub == "brightness")
    {
        std::string val = CommandParser::getCommandParameter(command, 2);
        uint8_t b = (uint8_t)strtoul(val.c_str(), NULL, 0);
        tft.setBrightness(b);
        loggerInstance->Info("Brightness set to " + std::to_string(b));
        return std::string("{\"event\":\"brightness\",\"value\":") + std::to_string(b) + "}";
    }

    loggerInstance->Info("Unknown screen subcommand: " + sub);
    return std::string("{\"event\":\"screen\",\"error\":\"unknown\",\"sub\":\"") + sub + "\"}";
}

// --- Page command handler ---

struct PageEntry
{
    const char *cmd;
    const char *appName;
};

static const PageEntry PAGE_TABLE[] = {
    {"rgb", "RGB LED"},       {"rgbled", "RGB LED"},  {"info", "Info"},
    {"wifi", "WiFi"},         {"sensors", "Sensors"}, {"display", "Display"},
    {"features", "Features"}, {"log", "Log Viewer"},  {"files", "File Manager"},
};

static std::string screenCommandHandler(const std::string &command)
{
    return UI::postToUITaskWithResult([&command]() -> std::string { return screenCommandHandlerImpl(command); });
}

static std::string pageCommandHandlerImpl(const std::string &command)
{
    std::string sub = CommandParser::getCommandParameter(command, 1);

    for (const auto &entry : PAGE_TABLE)
    {
        if (sub == entry.cmd)
        {
            UI::windowManager().openApp(entry.appName);
            loggerInstance->Info(std::string("Opening ") + entry.appName + " app");
            return std::string("{\"event\":\"page\", \"status\":\"success\", \"page\":\"") + sub + "\"}";
        }
    }

    loggerInstance->Info("Unknown page subcommand: " + sub);
    return std::string("{\"event\":\"page\",\"error\":\"unknown\",\"sub\":\"") + sub + "\"}";
}

// --- Window manager command handler ---

static std::string pageCommandHandler(const std::string &command)
{
    return UI::postToUITaskWithResult([&command]() -> std::string { return pageCommandHandlerImpl(command); });
}

static std::string wmCommandHandlerImpl(const std::string &command)
{
    std::string sub = CommandParser::getCommandParameter(command, 1);

    if (sub == "list")
    {
        auto &apps = UI::windowManager().getOpenApps();
        auto *focused = UI::windowManager().getFocused();
        auto *panel = UI::windowManager().getPanelSlot();
        std::string json = "{\"apps\":[";
        bool first = true;
        if (panel != nullptr)
        {
            json += "{\"name\":\"" + panel->name + "\",\"focused\":false,\"windowed\":false}";
            first = false;
        }
        for (size_t i = 0; i < apps.size(); i++)
        {
            if (!first)
            {
                json += ",";
            }
            first = false;
            bool isFocused = ((focused != nullptr) && &apps[i] == focused);
            json += R"({"name":")" + apps[i].name + "\",\"focused\":" + (isFocused ? "true" : "false") +
                    ",\"windowed\":true}";
        }
        json += "]}";
        return json;
    }

    if (sub == "focus")
    {
        std::string name = CommandParser::getCommandParameter(command, 2);
        if (name.empty())
        {
            return "{\"error\":\"No app name\"}";
        }
        UI::windowManager().restoreApp(name.c_str());
        return "{\"status\":\"ok\"}";
    }

    if (sub == "close")
    {
        std::string name = CommandParser::getCommandParameter(command, 2);
        if (name.empty())
        {
            return "{\"error\":\"No app name\"}";
        }
        UI::windowManager().closeApp(name.c_str());
        return "{\"status\":\"ok\"}";
    }

    if (sub == "keyboard")
    {
        UI::desktop().toggleKeyboard();
        return "{\"status\":\"ok\"}";
    }

    return "{\"error\":\"Usage: wm list | wm focus <name> | wm close <name> | wm keyboard\"}";
}

static std::string wmCommandHandler(const std::string &command)
{
    return UI::postToUITaskWithResult([&command]() -> std::string { return wmCommandHandlerImpl(command); });
}

// --- Action definitions ---

static FeatureAction wmAction = {.name = "wm",
                                 .handler = wmCommandHandler,
                                 .transports = {.cli = true, .rest = false, .ws = true, .scripting = true}};

static FeatureAction screenAction = {.name = "screen",
                                     .handler = screenCommandHandler,
                                     .transports = {.cli = true, .rest = false, .ws = true, .scripting = true}};

static FeatureAction pageAction = {.name = "page",
                                   .handler = pageCommandHandler,
                                   .transports = {.cli = true, .rest = false, .ws = true, .scripting = true}};

// --- Feature ---

static Feature *createuiFeature()
{
    auto *f = new Feature(
        "UI",
        []()
        {
            UI::initTaskQueue();

            tft.fillScreen(TFT_BLACK);
            tft.setTextColor(TFT_WHITE, TFT_BLACK);
            tft.setTextSize(1);
            tft.setCursor(0, 0);
            tft.println("UI Feature Initialized");

            readCalibrationData();

            if (!UI::initRenderer())
            {
                loggerInstance->Error("Failed to initialize renderer");
                return FeatureState::ERROR;
            }

            UI::desktop().init();

            actionRegistryInstance->registerAction(&screenAction);
            actionRegistryInstance->registerAction(&pageAction);
            actionRegistryInstance->registerAction(&wmAction);

            esp_timer_create_args_t args = {};
            args.callback = [](void *)
            {
                frameReady = true;
            };
            args.name = "ui_frame";
            esp_timer_create(&args, &frameTimer);
            esp_timer_start_periodic(frameTimer, 33000);

            loggerInstance->Info("UI feature initialized (Win95 desktop)");

            return FeatureState::RUNNING;
        },
        []()
        {
            if (!uiTaskInitDone)
            {
                UI::setUITaskHandle(xTaskGetCurrentTaskHandle());
                uiTaskInitDone = true;
            }

            UI::processTaskQueue();

            static bool prevTouched = false;
            static int prevX = 0;
            static int prevY = 0;
            int tx;
            int ty;
            bool touched = tft.getTouch(&tx, &ty);

            if (touched)
            {
                UI::desktop().handleTouch(tx, ty);
                UI::markDirty();
            }
            else if (prevTouched)
            {
                UI::desktop().handleTouchEnd(prevX, prevY);
                UI::executeQueuedActions();
                UI::markDirty();
            }

            prevTouched = touched;
            if (touched)
            {
                prevX = tx;
                prevY = ty;
            }

            UI::desktop().tickTimers();

            UI::executeQueuedActions();

            if (UI::isDirty() && frameReady)
            {
                frameReady = false;
                UI::desktop().draw();
                UI::clearDirty();
            }
        },
        []()
        {
            UI::setUITaskHandle(nullptr);
            uiTaskInitDone = false;

            if (frameTimer)
            {
                esp_timer_stop(frameTimer);
                esp_timer_delete(frameTimer);
                frameTimer = nullptr;
            }
        });

    f->configureTask(8192, 0, 2);
    return f;
}

Feature *uiFeature = createuiFeature();

#endif
