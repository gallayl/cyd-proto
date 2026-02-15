#include "../../../config.h"

#if ENABLE_UI

#include <Esp.h>
#include <esp_timer.h>
#include "UiFeature.h"
#include "../Logging.h"
#include "../../../ActionRegistry/ActionRegistry.h"
#include "ActionQueue.h"
#include "Renderer.h"
#include "Desktop.h"
#include "WindowManager.h"

static volatile bool frameReady = true;
static esp_timer_handle_t frameTimer = nullptr;

// --- Screen command handler ---

static String screenCommandHandler(const String &command)
{
    String sub = CommandParser::GetCommandParameter(command, 1);

    if (sub == "calibrate")
    {
        calibrateScreen();
        LoggerInstance->Info(F("Calibrated touch screen"));
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

        LoggerInstance->Info(F("Displayed hello world demo"));
        return String("{\"event\":\"helloDemo\"}");
    }
    else if (sub == "rotate")
    {
        String rotateParam = CommandParser::GetCommandParameter(command, 2);
        uint16_t rotate = 0;
        if (rotateParam.length() > 0)
        {
            rotate = (uint16_t)strtoul(rotateParam.c_str(), NULL, 0);
        }
        tft.setRotation(rotate);
        UI::reinitRenderer();
        readCalibrationData();
        UI::windowManager().relayoutAll();
        LoggerInstance->Info("Screen rotated to " + String(rotate));
        return String(String("{\"event\":\"rotate\",\"rotation\":") + rotate + String("}"));
    }
    else if (sub == "clear")
    {
        String colorParam = CommandParser::GetCommandParameter(command, 2);
        uint16_t color = TFT_BLACK;
        if (colorParam.length() > 0)
        {
            color = (uint16_t)strtoul(colorParam.c_str(), NULL, 0);
        }
        tft.fillScreen(color);
        LoggerInstance->Info(F("Screen cleared"));
        return String(String("{\"event\":\"clear\",\"color\":\"") + colorParam + String("\"}"));
    }
    else if (sub == "text")
    {
        int x = CommandParser::GetCommandParameter(command, 2).toInt();
        int y = CommandParser::GetCommandParameter(command, 3).toInt();
        uint16_t fg = (uint16_t)strtoul(CommandParser::GetCommandParameter(command, 4).c_str(), NULL, 0);
        uint16_t bg = (uint16_t)strtoul(CommandParser::GetCommandParameter(command, 5).c_str(), NULL, 0);
        int sz = CommandParser::GetCommandParameter(command, 6).toInt();
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
        LoggerInstance->Info("Drew text: " + msg);
        return String("{\"event\":\"text\"}");
    }
    else if (sub == "pixel")
    {
        int x = CommandParser::GetCommandParameter(command, 2).toInt();
        int y = CommandParser::GetCommandParameter(command, 3).toInt();
        uint16_t color = (uint16_t)strtoul(CommandParser::GetCommandParameter(command, 4).c_str(), NULL, 0);
        tft.drawPixel(x, y, color);
        LoggerInstance->Info("Drew pixel at " + String(x) + "," + String(y));
        return String("{\"event\":\"pixel\"}");
    }
    else if (sub == "rect")
    {
        int x = CommandParser::GetCommandParameter(command, 2).toInt();
        int y = CommandParser::GetCommandParameter(command, 3).toInt();
        int w = CommandParser::GetCommandParameter(command, 4).toInt();
        int h = CommandParser::GetCommandParameter(command, 5).toInt();
        uint16_t color = (uint16_t)strtoul(CommandParser::GetCommandParameter(command, 6).c_str(), NULL, 0);
        tft.drawRect(x, y, w, h, color);
        LoggerInstance->Info("Drew rect at " + String(x) + "," + String(y));
        return String("{\"event\":\"rect\"}");
    }
    else if (sub == "fillrect")
    {
        int x = CommandParser::GetCommandParameter(command, 2).toInt();
        int y = CommandParser::GetCommandParameter(command, 3).toInt();
        int w = CommandParser::GetCommandParameter(command, 4).toInt();
        int h = CommandParser::GetCommandParameter(command, 5).toInt();
        uint16_t color = (uint16_t)strtoul(CommandParser::GetCommandParameter(command, 6).c_str(), NULL, 0);
        tft.fillRect(x, y, w, h, color);
        LoggerInstance->Info("Drew filled rect at " + String(x) + "," + String(y));
        return String("{\"event\":\"fillrect\"}");
    }
    else if (sub == "circle")
    {
        int x = CommandParser::GetCommandParameter(command, 2).toInt();
        int y = CommandParser::GetCommandParameter(command, 3).toInt();
        int r = CommandParser::GetCommandParameter(command, 4).toInt();
        uint16_t color = (uint16_t)strtoul(CommandParser::GetCommandParameter(command, 5).c_str(), NULL, 0);
        tft.drawCircle(x, y, r, color);
        LoggerInstance->Info("Drew circle at " + String(x) + "," + String(y));
        return String("{\"event\":\"circle\"}");
    }
    else if (sub == "fillcircle")
    {
        int x = CommandParser::GetCommandParameter(command, 2).toInt();
        int y = CommandParser::GetCommandParameter(command, 3).toInt();
        int r = CommandParser::GetCommandParameter(command, 4).toInt();
        uint16_t color = (uint16_t)strtoul(CommandParser::GetCommandParameter(command, 5).c_str(), NULL, 0);
        tft.fillCircle(x, y, r, color);
        LoggerInstance->Info("Drew filled circle at " + String(x) + "," + String(y));
        return String("{\"event\":\"fillcircle\"}");
    }
    else if (sub == "brightness")
    {
        String val = CommandParser::GetCommandParameter(command, 2);
        uint8_t b = (uint8_t)strtoul(val.c_str(), NULL, 0);
        tft.setBrightness(b);
        LoggerInstance->Info("Brightness set to " + String(b));
        return String(String("{\"event\":\"brightness\",\"value\":") + b + String("}"));
    }

    LoggerInstance->Info("Unknown screen subcommand: " + sub);
    return String(String("{\"event\":\"screen\",\"error\":\"unknown\",\"sub\":\"") + sub + String("\"}"));
}

// --- Page command handler ---

struct PageEntry
{
    const char *cmd;
    const char *appName;
};

static const PageEntry pageTable[] = {
    {"rgb", "RGB LED"},
    {"rgbled", "RGB LED"},
    {"info", "Info"},
    {"wifi", "WiFi"},
    {"sensors", "Sensors"},
    {"display", "Display"},
    {"features", "Features"},
    {"log", "Log Viewer"},
    {"files", "File Manager"},
};

static String pageCommandHandler(const String &command)
{
    String sub = CommandParser::GetCommandParameter(command, 1);

    for (auto &entry : pageTable)
    {
        if (sub == entry.cmd)
        {
            UI::windowManager().openApp(entry.appName);
            LoggerInstance->Info(String("Opening ") + entry.appName + " app");
            return String("{\"event\":\"page\", \"status\":\"success\", \"page\":\"") + sub + "\"}";
        }
    }

    LoggerInstance->Info("Unknown page subcommand: " + sub);
    return String(String("{\"event\":\"page\",\"error\":\"unknown\",\"sub\":\"") + sub + String("\"}"));
}

// --- Window manager command handler ---

static String wmCommandHandler(const String &command)
{
    String sub = CommandParser::GetCommandParameter(command, 1);

    if (sub == "list")
    {
        auto &apps = UI::windowManager().getOpenApps();
        auto *focused = UI::windowManager().getFocused();
        String json = "{\"apps\":[";
        for (size_t i = 0; i < apps.size(); i++)
        {
            if (i > 0)
                json += ",";
            bool isFocused = (focused && &apps[i] == focused);
            json += "{\"name\":\"" + apps[i].name + "\",\"focused\":" + (isFocused ? "true" : "false") + "}";
        }
        json += "]}";
        return json;
    }

    if (sub == "focus")
    {
        String name = CommandParser::GetCommandParameter(command, 2);
        if (name.length() == 0)
            return String(F("{\"error\":\"No app name\"}"));
        UI::windowManager().restoreApp(name.c_str());
        return String(F("{\"status\":\"ok\"}"));
    }

    if (sub == "close")
    {
        String name = CommandParser::GetCommandParameter(command, 2);
        if (name.length() == 0)
            return String(F("{\"error\":\"No app name\"}"));
        UI::windowManager().closeApp(name.c_str());
        return String(F("{\"status\":\"ok\"}"));
    }

    if (sub == "start_menu")
    {
        UI::desktop().toggleStartMenu();
        return String(F("{\"status\":\"ok\"}"));
    }

    if (sub == "keyboard")
    {
        UI::desktop().toggleKeyboard();
        return String(F("{\"status\":\"ok\"}"));
    }

    return String(F("{\"error\":\"Usage: wm list | wm focus <name> | wm close <name> | wm start_menu | wm keyboard\"}"));
}

// --- Action definitions ---

FeatureAction wmAction = {
    .name = "wm",
    .handler = wmCommandHandler,
    .transports = {.cli = true, .rest = false, .ws = true, .scripting = true}};

FeatureAction screenAction = {
    .name = "screen",
    .handler = screenCommandHandler,
    .transports = {.cli = true, .rest = false, .ws = true, .scripting = true}};

FeatureAction pageAction = {
    .name = "page",
    .handler = pageCommandHandler,
    .transports = {.cli = true, .rest = false, .ws = true, .scripting = true}};

// --- Feature ---

Feature *UiFeature = new Feature("UI", []()
                                 {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(1);
    tft.setCursor(0, 0);
    tft.println("UI Feature Initialized");

    readCalibrationData();

    if (!UI::initRenderer())
    {
        LoggerInstance->Error(F("Failed to create sprite back-buffer (low memory?)"));
        LoggerInstance->Error("Free heap: " + String(ESP.getFreeHeap()) + " bytes");
        return FeatureState::ERROR;
    }

    UI::desktop().init();

    ActionRegistryInstance->RegisterAction(&screenAction);
    ActionRegistryInstance->RegisterAction(&pageAction);
    ActionRegistryInstance->RegisterAction(&wmAction);

    esp_timer_create_args_t args = {};
    args.callback = [](void *) { frameReady = true; };
    args.name = "ui_frame";
    esp_timer_create(&args, &frameTimer);
    esp_timer_start_periodic(frameTimer, 33000);

    LoggerInstance->Info(F("UI feature initialized (Win95 desktop)"));

    return FeatureState::RUNNING; }, []()
                                 {
    static bool prevTouched = false;
    static int prevX = 0, prevY = 0;
    int tx, ty;
    bool touched = tft.getTouch(&tx, &ty);

    if (touched) {
        UI::desktop().handleTouch(tx, ty);
        UI::markDirty();
    } else if (prevTouched) {
        UI::desktop().handleTouchEnd(prevX, prevY);
        UI::executeQueuedActions();
        UI::markDirty();
    }

    prevTouched = touched;
    if (touched) {
        prevX = tx;
        prevY = ty;
    }

    UI::desktop().tickTimers();

    if (UI::isDirty() && frameReady) {
        frameReady = false;
        UI::desktop().draw();
        UI::clearDirty();
    } }, []()
                                 {
    if (frameTimer) {
        esp_timer_stop(frameTimer);
        esp_timer_delete(frameTimer);
        frameTimer = nullptr;
    } });

#endif
