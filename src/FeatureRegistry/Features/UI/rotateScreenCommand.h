#pragma once

#include <stdlib.h>
#include <LovyanGFX.hpp>
#include "../../../hw/Screen.h"
#include "../../../CommandInterpreter/CustomCommand.h"
#include "../../../CommandInterpreter/CommandParser.h"
#include "../Logging.h"

CustomCommand *rotateScreenCustomCommand = new CustomCommand("rotate", [](String command)
                                                             {
                                                                String rotateParam = CommandParser::GetCommandParameter(command, 1);
                                                                uint16_t rotate = 0;
                                                                if (rotateParam.length() > 0)
                                                                {
                                                                    // allow decimal or hex (0x...)
                                                                    rotate = (uint16_t)strtoul(rotateParam.c_str(), NULL, 0);
                                                                }
                                                                tft.setRotation(rotate);
                                                                LoggerInstance->Info("Screen rotated to " + String(rotate));
                                                                return String(String("{\"event\":\"rotate\",\"rotation\":") + rotate + String("}")); });