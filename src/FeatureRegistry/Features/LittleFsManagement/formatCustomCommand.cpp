#include "formatCustomCommand.h"

CustomCommand *formatCustomCommand = new CustomCommand("format", [](const String &command)
                                                       {
    LittleFS.format();
    return String("{\"event\": \"format\"}"); });