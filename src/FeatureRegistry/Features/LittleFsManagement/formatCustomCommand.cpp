#include "formatCustomCommand.h"

CustomCommand *formatCustomCommand = new CustomCommand("format", [](String command)
                                                       {
    LittleFS.format();
    return String("{\"event\": \"format\"}");
});