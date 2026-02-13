#pragma once

#include "../../../CommandInterpreter/CustomCommand.h"
#include <LittleFS.h>

CustomCommand *formatCustomCommand = new CustomCommand("format", [](String command)
                                                       {
    LittleFS.format();
    return String("{\"event\": \"format\"}"); });