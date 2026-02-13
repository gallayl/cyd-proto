#include "rgbLedCommand.h"

CustomCommand *rgbLedCustomCommand = new CustomCommand("rgbLed", [](String command)
                                                       {
                                                           const String sub = CommandParser::GetCommandParameter(command, 1);
                                                           if (sub == "setColor")
                                                           {
                                                               int r = CommandParser::GetCommandParameter(command, 2).toInt();
                                                               int g = CommandParser::GetCommandParameter(command, 3).toInt();
                                                               int b = CommandParser::GetCommandParameter(command, 4).toInt();
                                                               setRgbLedColor(r, g, b);
                                                               LoggerInstance->Info("Set RGB LED color to " + String(r) + "," + String(g) + "," + String(b));
                                                           }
                                                           else if (sub == "off")
                                                           {
                                                               setRgbLedColor(0, 0, 0);
                                                               LoggerInstance->Info("Turned off RGB LED");
                                                           }
                                                           else
                                                           {
                                                               LoggerInstance->Error("Unknown rgbLed subcommand: " + sub);
                                                           }
                                                           return String("{\"event\": \"rgbLedCommandExecuted\"}");
                                                       });