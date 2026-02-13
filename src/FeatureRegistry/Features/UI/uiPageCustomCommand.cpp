#include "uiPageCustomCommand.h"
#include "./screens/welcome.h"
#include "./screens/rgbled.h"

extern LGFX tft;

static String pageCommandHandler(const String &command)
{
    String sub = CommandParser::GetCommandParameter(command, 1);

    if (sub == "welcome")
    {
        showWelcomeScreen();
        LoggerInstance->Info("Showing welcome page");
        return String("{\"event\":\"page\", \"status\":\"success\", \"page\":\"welcome\"}");
    }

    if (sub == "rgb" || sub == "rgbled")
    {
        showRgbLedScreen();
        LoggerInstance->Info("Showing rgbled page");
        return String("{\"event\":\"page\", \"status\":\"success\", \"page\":\"rgb\"}");
    }

    // unknown subcommand
    LoggerInstance->Info("Unknown page subcommand: " + sub);
    return String(String("{\"event\":\"page\",\"error\":\"unknown\",\"sub\":\"") + sub + String("\"}"));
}

CustomCommand *pageCustomCommand = new CustomCommand("page", [](const String &command)
                                                     { return pageCommandHandler(command); });