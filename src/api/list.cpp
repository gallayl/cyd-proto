#include "list.h"
#include "../fs/VirtualFS.h"
#include "../mime.h"
#include <string>

#ifdef USE_ESP_IDF

#include <dirent.h>
#include <sys/stat.h>
#include <cstring>

cJSON *getFileList()
{
    return getFileList(resolveToLittleFsPath("/"));
}

cJSON *getFileList(const char *path)
{
    return getFileList(resolveToLittleFsPath(path));
}

cJSON *getFileList(const std::string &realPath)
{
    cJSON *fileList = cJSON_CreateArray();

    DIR *dir = opendir(realPath.c_str());
    if (!dir)
    {
        return fileList;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        std::string entryPath = realPath;
        if (entryPath.back() != '/')
        {
            entryPath += '/';
        }
        entryPath += entry->d_name;

        struct stat st;
        bool isDir = (entry->d_type == DT_DIR);
        size_t fileSize = 0;
        time_t lastWrite = 0;

        if (stat(entryPath.c_str(), &st) == 0)
        {
            isDir = S_ISDIR(st.st_mode);
            fileSize = st.st_size;
            lastWrite = st.st_mtime;
        }

        cJSON *o = cJSON_CreateObject();
        cJSON_AddStringToObject(o, "name", entry->d_name);
        cJSON_AddNumberToObject(o, "size", fileSize);
        cJSON_AddBoolToObject(o, "isDir", isDir);
        cJSON_AddStringToObject(o, "path", entryPath.c_str());
        cJSON_AddNumberToObject(o, "lastWrite", (double)lastWrite);
        cJSON_AddItemToArray(fileList, o);
    }

    closedir(dir);
    return fileList;
}

#else // Arduino

#include <LittleFS.h>
#include <ArduinoJson.h>

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
        std::string path(request->getParam("path")->value().c_str());
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

    std::string responseStr;
    serializeJson(response, responseStr);
    request->send(200, MIME_JSON, responseStr.c_str()); });

#endif // USE_ESP_IDF
