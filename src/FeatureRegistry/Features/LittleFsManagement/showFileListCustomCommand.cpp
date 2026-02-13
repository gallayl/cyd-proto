#include "showFileListCustomCommand.h"

CustomCommand *showFileListCustomCommand = new CustomCommand("list", [](const String &command)
                                                             {
    JsonDocument response = getFileList();
    String output;
    serializeJson(response, output);
    return output; });