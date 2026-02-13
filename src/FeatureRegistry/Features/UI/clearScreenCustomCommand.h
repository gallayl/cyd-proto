#pragma once

#include <stdlib.h>
#include <TFT_eSPI.h>
#include "../../../CommandInterpreter/CustomCommand.h"
#include "../../../CommandInterpreter/CommandParser.h"
#include "../Logging.h"

// the TFT object is defined in UiFeature.h
extern TFT_eSPI tft;

CustomCommand *clearScreenCustomCommand = new CustomCommand("info", [](String command)
                                                            {
                                                                String colorParam = CommandParser::GetCommandParameter(command, 1);
                                                                uint16_t color = TFT_BLACK;
                                                                if (colorParam.length() > 0)
                                                                {
                                                                    // allow decimal or hex (0x...)
                                                                    color = (uint16_t)strtoul(colorParam.c_str(), NULL, 0);
                                                                }
                                                                tft.fillScreen(color);
                                                                LoggerInstance->Info("Screen cleared");
                                                                return String(String("{\"event\":\"clear\",\"color\":") + colorParam + String("}"));
                                                            });