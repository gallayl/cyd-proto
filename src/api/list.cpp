#include "list.h"
#include <LittleFS.h>
#include "../fs/VirtualFS.h"
#include "../mime.h"

JsonDocument getFileList()
{
    return getFileList(LittleFS, "/");
}

JsonDocument getFileList(const char *path)
{
    return getFileList(LittleFS, path);
}

JsonDocument getFileList(fs::FS &filesystem, const char *path)
{
    fs::File root = filesystem.open(path, "r");
    JsonDocument response;
    JsonArray fileList = response.to<JsonArray>();

    if (!root || !root.isDirectory())
    {
        return response;
    };

#ifdef ESP32
    File file = root.openNextFile("r");
#else
    File file = root.openNextFile();
#endif
    while (file)
    {
        JsonObject o = fileList.add<JsonObject>();
        o["name"] = file.name();
        o["size"] = file.size();
        o["isDir"] = file.isDirectory();
#ifdef ESP32
        o["path"] = file.path();
#endif
        o["lastWrite"] = file.getLastWrite();

#ifdef ESP32
        file = root.openNextFile("r");
#else
        file = root.openNextFile();
#endif
    }
    return response;
}

ArRequestHandlerFunction listFiles = ([](AsyncWebServerRequest *request)
                                      {
    JsonDocument response;

    if (request->hasParam("path"))
    {
        String path = request->getParam("path")->value();
        ResolvedPath resolved = resolveVirtualPath(path);
        if (resolved.valid && resolved.fs) {
            response = getFileList(*resolved.fs, resolved.localPath.c_str());
        } else {
            response = getFileList(LittleFS, path.c_str());
        }
    }
    else
    {
        response = getFileList();
    }

    String responseStr;
    serializeJson(response, responseStr);
    request->send(200, MIME_JSON, responseStr); });
