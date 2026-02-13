#include "showFileListCustomCommand.h"
#include "../../../config.h"

CustomCommand *showFileListCustomCommand = new CustomCommand("list", [](String command)
                                                             {
    JsonDocument response;

    File root = LittleFS.open("/", "r");

    if (!root)
    {
        LoggerInstance->Error(F("Failed to open directory"));
        return String("");
    }

    JsonArray files = response.to<JsonArray>();

    while (File file = root.openNextFile())
    {
        JsonObject fileObject = files.add<JsonObject>();
        fileObject["name"] = file.name();
        fileObject["size"] = file.size();
        fileObject["isDirectory"] = file.isDirectory();
        file.close();
    }

    char buffer[JSON_BUFFER_SIZE];
    serializeJson(response, buffer);

    root.close();

    return String(buffer);
});