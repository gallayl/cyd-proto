#pragma once

#include <stdlib.h>
#include <LovyanGFX.hpp>
#include "./LovyanGFX_ILI9341_Settings.h"
#include "../../../CommandInterpreter/CustomCommand.h"
#include "../../../CommandInterpreter/CommandParser.h"
#include "../Logging.h"

// the TFT object is defined in UiFeature.h
extern LGFX tft;

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