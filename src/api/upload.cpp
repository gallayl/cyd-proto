#include "upload.h"
#include <LittleFS.h>
#include "../mime.h" // for MIME_plainText/MIME_json

ArRequestHandlerFunction onPostUploadFiles = ([](AsyncWebServerRequest *request) {});

static String sanitizeFilename(const String &raw)
{
    String name = raw;
    // strip any path separators and parent-directory references
    name.replace("..", "");
    name.replace("/", "");
    name.replace("\\", "");
    name.trim();
    if (name.length() == 0)
        name = "unnamed";
    return name;
}

ArUploadHandlerFunction uploadFiles = ([](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
                                       {
        String safeName = sanitizeFilename(filename);
        fs::File file = LittleFS.open("/" + safeName, index == 0 ? "w" : "a");

        if (!file)
        {
            LoggerInstance->Error(F("Failed to open file for writing"));
            request->send(500, MIME_json, "{\"error\":\"Failed to open file for writing\"}");
            return;
        }
        file.write(data, len);
        file.close();

    if (final)
    {
        LoggerInstance->Info(F("Upload finished"));
        request->send(200, MIME_json, "{\"status\":\"ok\"}");
    }; });