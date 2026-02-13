#include "uiPageCustomCommand.h"
#include "WindowManager.h"

extern LGFX tft;

static String pageCommandHandler(const String &command)
{
    String sub = CommandParser::GetCommandParameter(command, 1);

    if (sub == "rgb" || sub == "rgbled")
    {
        UI::windowManager().openApp("RGB LED");
        LoggerInstance->Info("Opening RGB LED app");
        return String("{\"event\":\"page\", \"status\":\"success\", \"page\":\"rgb\"}");
    }

    if (sub == "info")
    {
        UI::windowManager().openApp("Info");
        LoggerInstance->Info("Opening Info app");
        return String("{\"event\":\"page\", \"status\":\"success\", \"page\":\"info\"}");
    }

    if (sub == "wifi")
    {
        UI::windowManager().openApp("WiFi");
        LoggerInstance->Info("Opening WiFi app");
        return String("{\"event\":\"page\", \"status\":\"success\", \"page\":\"wifi\"}");
    }

    LoggerInstance->Info("Unknown page subcommand: " + sub);
    return String(String("{\"event\":\"page\",\"error\":\"unknown\",\"sub\":\"") + sub + String("\"}"));
}

CustomCommand *pageCustomCommand = new CustomCommand("page", [](const String &command)
                                                     { return pageCommandHandler(command); });
