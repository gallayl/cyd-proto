#include "screenCustomCommand.h"

// the TFT object is defined elsewhere (UiFeature.cpp)
extern LGFX tft;

static String screenCommandHandler(String command)
{
    String sub = CommandParser::GetCommandParameter(command, 1);

    if (sub == "calibrate")
    {
        calibrateScreen();
        LoggerInstance->Info("Calibrated touch screen");
        return String("{\"event\":\"calibrate\", \"status\":\"success\"}");
    }
    else if (sub == "demo")
    {
        // simple hello world demo on the TFT
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(0, 0);
        tft.setTextColor(TFT_GREENYELLOW);
        tft.setTextSize(2);
        tft.println("Hello World!");

        tft.drawCircle(120, 160, 50, TFT_MAGENTA);
        tft.drawEllipse(200, 160, 60, 40, TFT_GOLD);

        LoggerInstance->Info("Displayed hello world demo");
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
        LoggerInstance->Info("Screen cleared");
        return String(String("{\"event\":\"clear\",\"color\":\"") + colorParam + String("\"}"));
    }
    else if (sub == "text")
    {
        int x = CommandParser::GetCommandParameter(command, 2).toInt();
        int y = CommandParser::GetCommandParameter(command, 3).toInt();
        uint16_t fg = (uint16_t)strtoul(CommandParser::GetCommandParameter(command, 4).c_str(), NULL, 0);
        uint16_t bg = (uint16_t)strtoul(CommandParser::GetCommandParameter(command, 5).c_str(), NULL, 0);
        int sz = CommandParser::GetCommandParameter(command, 6).toInt();
        // remainder of the command is the text to draw
        int count = 0;
        int pos = -1;
        for (int i = 0; i < command.length(); ++i)
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

    // unknown subcommand
    LoggerInstance->Info("Unknown screen subcommand: " + sub);
    return String(String("{\"event\":\"screen\",\"error\":\"unknown\",\"sub\":\"") + sub + String("\"}"));
}

CustomCommand *screenCustomCommand = new CustomCommand("screen", [](String command)
                                                       { return screenCommandHandler(command); });