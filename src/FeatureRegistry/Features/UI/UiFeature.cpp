#include "../../../config.h"

#if ENABLE_UI

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

static String screenCommandHandlerImpl(const String &command)
{
    String sub = CommandParser::getCommandParameter(command, 1);

    if (sub == "calibrate")
    {
        calibrateScreen();
        delay(1000); // Let user see "Calibration complete!" message
        tft.fillScreen(TFT_BLACK);
        tft.waitDisplay(); // Wait for fillScreen to complete
        if (!UI::reinitRenderer())
        {
            loggerInstance->Error(F("Failed to reinit renderer after calibration"));
            return String("{\"event\":\"calibrate\", \"status\":\"error\", \"message\":\"renderer_init_failed\"}");
        }
        UI::windowManager().relayoutAll();
        frameReady = true; // Ensure next frame will render
        UI::markDirty();   // Ensure UI knows it needs redraw
        loggerInstance->Info(F("Calibrated touch screen"));
        return String("{\"event\":\"calibrate\", \"status\":\"success\"}");
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

        loggerInstance->Info(F("Displayed hello world demo"));
        return String("{\"event\":\"helloDemo\"}");
    }
    else if (sub == "rotate")
    {
        String rotateParam = CommandParser::getCommandParameter(command, 2);
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
            loggerInstance->Error(F("Failed to reinit renderer after rotation"));
            return String(String("{\"event\":\"rotate\",\"rotation\":") + rotate +
                          String(",\"error\":\"renderer_init_failed\"}"));
        }
        readCalibrationData(); // Loads rotation-specific calibration
        UI::windowManager().relayoutAll();
        frameReady = true; // Ensure next frame will render
        UI::markDirty();   // Ensure UI knows it needs redraw
        loggerInstance->Info("Screen rotated to " + String(rotate));
        return String(String("{\"event\":\"rotate\",\"rotation\":") + rotate + String("}"));
    }
    else if (sub == "clear")
    {
        String colorParam = CommandParser::getCommandParameter(command, 2);
        uint16_t color = TFT_BLACK;
        if (colorParam.length() > 0)
        {
            color = (uint16_t)strtoul(colorParam.c_str(), NULL, 0);
        }
        tft.fillScreen(color);
        loggerInstance->Info(F("Screen cleared"));
        return String(String("{\"event\":\"clear\",\"color\":\"") + colorParam + String("\"}"));
    }
    else if (sub == "text")
    {
        int x = CommandParser::getCommandParameter(command, 2).toInt();
        int y = CommandParser::getCommandParameter(command, 3).toInt();
        uint16_t fg = (uint16_t)strtoul(CommandParser::getCommandParameter(command, 4).c_str(), NULL, 0);
        uint16_t bg = (uint16_t)strtoul(CommandParser::getCommandParameter(command, 5).c_str(), NULL, 0);
        int sz = CommandParser::getCommandParameter(command, 6).toInt();
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
        String msg = (pos >= 0) ? command.substring(pos + 1) : String();
        tft.setCursor(x, y);
        tft.setTextColor(fg, bg);
        tft.setTextSize(sz);
        tft.print(msg);
        loggerInstance->Info("Drew text: " + msg);
        return String("{\"event\":\"text\"}");
    }
    else if (sub == "pixel")
    {
        int x = CommandParser::getCommandParameter(command, 2).toInt();
        int y = CommandParser::getCommandParameter(command, 3).toInt();
        uint16_t color = (uint16_t)strtoul(CommandParser::getCommandParameter(command, 4).c_str(), NULL, 0);
        tft.drawPixel(x, y, color);
        loggerInstance->Info("Drew pixel at " + String(x) + "," + String(y));
        return String("{\"event\":\"pixel\"}");
    }
    else if (sub == "rect")
    {
        int x = CommandParser::getCommandParameter(command, 2).toInt();
        int y = CommandParser::getCommandParameter(command, 3).toInt();
        int w = CommandParser::getCommandParameter(command, 4).toInt();
        int h = CommandParser::getCommandParameter(command, 5).toInt();
        uint16_t color = (uint16_t)strtoul(CommandParser::getCommandParameter(command, 6).c_str(), NULL, 0);
        tft.drawRect(x, y, w, h, color);
        loggerInstance->Info("Drew rect at " + String(x) + "," + String(y));
        return String("{\"event\":\"rect\"}");
    }
    else if (sub == "fillrect")
    {
        int x = CommandParser::getCommandParameter(command, 2).toInt();
        int y = CommandParser::getCommandParameter(command, 3).toInt();
        int w = CommandParser::getCommandParameter(command, 4).toInt();
        int h = CommandParser::getCommandParameter(command, 5).toInt();
        uint16_t color = (uint16_t)strtoul(CommandParser::getCommandParameter(command, 6).c_str(), NULL, 0);
        tft.fillRect(x, y, w, h, color);
        loggerInstance->Info("Drew filled rect at " + String(x) + "," + String(y));
        return String("{\"event\":\"fillrect\"}");
    }
    else if (sub == "circle")
    {
        int x = CommandParser::getCommandParameter(command, 2).toInt();
        int y = CommandParser::getCommandParameter(command, 3).toInt();
        int r = CommandParser::getCommandParameter(command, 4).toInt();
        uint16_t color = (uint16_t)strtoul(CommandParser::getCommandParameter(command, 5).c_str(), NULL, 0);
        tft.drawCircle(x, y, r, color);
        loggerInstance->Info("Drew circle at " + String(x) + "," + String(y));
        return String("{\"event\":\"circle\"}");
    }
    else if (sub == "fillcircle")
    {
        int x = CommandParser::getCommandParameter(command, 2).toInt();
        int y = CommandParser::getCommandParameter(command, 3).toInt();
        int r = CommandParser::getCommandParameter(command, 4).toInt();
        uint16_t color = (uint16_t)strtoul(CommandParser::getCommandParameter(command, 5).c_str(), NULL, 0);
        tft.fillCircle(x, y, r, color);
        loggerInstance->Info("Drew filled circle at " + String(x) + "," + String(y));
        return String("{\"event\":\"fillcircle\"}");
    }
    else if (sub == "brightness")
    {
        String val = CommandParser::getCommandParameter(command, 2);
        uint8_t b = (uint8_t)strtoul(val.c_str(), NULL, 0);
        tft.setBrightness(b);
        loggerInstance->Info("Brightness set to " + String(b));
        return String(String("{\"event\":\"brightness\",\"value\":") + b + String("}"));
    }

    loggerInstance->Info("Unknown screen subcommand: " + sub);
    return String(String("{\"event\":\"screen\",\"error\":\"unknown\",\"sub\":\"") + sub + String("\"}"));
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

static String screenCommandHandler(const String &command)
{
    return UI::postToUITaskWithResult([&command]() -> String { return screenCommandHandlerImpl(command); });
}

static String pageCommandHandlerImpl(const String &command)
{
    String sub = CommandParser::getCommandParameter(command, 1);

    for (const auto &entry : PAGE_TABLE)
    {
        if (sub == entry.cmd)
        {
            UI::windowManager().openApp(entry.appName);
            loggerInstance->Info(String("Opening ") + entry.appName + " app");
            return String("{\"event\":\"page\", \"status\":\"success\", \"page\":\"") + sub + "\"}";
        }
    }

    loggerInstance->Info("Unknown page subcommand: " + sub);
    return String(String("{\"event\":\"page\",\"error\":\"unknown\",\"sub\":\"") + sub + String("\"}"));
}

// --- Window manager command handler ---

static String pageCommandHandler(const String &command)
{
    return UI::postToUITaskWithResult([&command]() -> String { return pageCommandHandlerImpl(command); });
}

static String wmCommandHandlerImpl(const String &command)
{
    String sub = CommandParser::getCommandParameter(command, 1);

    if (sub == "list")
    {
        auto &apps = UI::windowManager().getOpenApps();
        auto *focused = UI::windowManager().getFocused();
        auto *panel = UI::windowManager().getPanelSlot();
        String json = "{\"apps\":[";
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
        String name = CommandParser::getCommandParameter(command, 2);
        if (name.length() == 0)
        {
            return String(F("{\"error\":\"No app name\"}"));
        }
        UI::windowManager().restoreApp(name.c_str());
        return String(F("{\"status\":\"ok\"}"));
    }

    if (sub == "close")
    {
        String name = CommandParser::getCommandParameter(command, 2);
        if (name.length() == 0)
        {
            return String(F("{\"error\":\"No app name\"}"));
        }
        UI::windowManager().closeApp(name.c_str());
        return String(F("{\"status\":\"ok\"}"));
    }

    if (sub == "keyboard")
    {
        UI::desktop().toggleKeyboard();
        return String(F("{\"status\":\"ok\"}"));
    }

    return String(F("{\"error\":\"Usage: wm list | wm focus <name> | wm close <name> | wm keyboard\"}"));
}

static String wmCommandHandler(const String &command)
{
    return UI::postToUITaskWithResult([&command]() -> String { return wmCommandHandlerImpl(command); });
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
                loggerInstance->Error(F("Failed to initialize renderer"));
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

            loggerInstance->Info(F("UI feature initialized (Win95 desktop)"));

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
