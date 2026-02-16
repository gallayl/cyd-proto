#include "upload.h"
#include "../mime.h"
#include "../fs/VirtualFS.h"
#include "../FeatureRegistry/Features/Logging.h"
#include "../utils/StringUtil.h"
#include <Arduino.h>
#include <string>

ArRequestHandlerFunction onPostUploadFiles = ([](AsyncWebServerRequest *request) {
    // Check if there was an error during upload (stored in _tempObject)
    if (request->_tempObject != nullptr) {
        std::string* errorMsg = (std::string*)request->_tempObject;
        request->send(400, MIME_JSON, errorMsg->c_str());
        delete errorMsg;
        request->_tempObject = nullptr;
        return;
    }
    request->send(200, MIME_JSON, "{\"status\":\"ok\"}");
});

static std::string sanitizePath(const std::string &raw)
{
    std::string path = raw;
    StringUtil::replaceAll(path, "\\", "/");
    path = StringUtil::trim(path);

    // Remove ".." segments to prevent directory traversal
    while (path.find("..") != std::string::npos)
    {
        StringUtil::replaceAll(path, "..", "");
    }

    // Collapse double slashes
    while (path.find("//") != std::string::npos)
    {
        StringUtil::replaceAll(path, "//", "/");
    }

    // Ensure path starts with /
    if (!StringUtil::startsWith(path, "/"))
    {
        path = std::string("/") + path;
    }

    // Remove trailing slash (unless it's the root)
    if (path.length() > 1 && StringUtil::endsWith(path, "/"))
    {
        path.erase(path.length() - 1);
    }

    if (path.length() <= 1)
    {
        return "";
    }

    return path;
}

static bool mkdirs(fs::FS &fs, const std::string &filePath)
{
    size_t idx = 1; // skip leading '/'
    while (true)
    {
        size_t slash = filePath.find('/', idx);
        if (slash == std::string::npos)
        {
            break;
        }
        std::string dir = filePath.substr(0, slash);
        fs.mkdir(dir.c_str());
        idx = slash + 1;
    }
    return true;
}

ArUploadHandlerFunction uploadFiles = ([](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final)
                                       {
    // If there's already an error, skip further processing
    if (request->_tempObject != nullptr) {
        return;
    }

    std::string targetPath(request->arg("path").c_str());

    if (targetPath.empty()) {
        std::string basePath(request->arg("basePath").c_str());
        if (!basePath.empty()) {
            if (!StringUtil::endsWith(basePath, "/")) {
                basePath += "/";
            }
            targetPath = basePath + filename.c_str();
        } else {
            targetPath = filename.c_str();
        }
    }

    std::string safePath = sanitizePath(targetPath);

    if (safePath.empty())
    {
        request->_tempObject = new std::string("{\"error\":\"Invalid file path\"}");
        return;
    }

    ResolvedPath resolved = resolveVirtualPath(safePath);
    if (!resolved.valid || !resolved.fs)
    {
        request->_tempObject = new std::string("{\"error\":\"Path must start with /flash or /sd\"}");
        return;
    }

    if (index == 0) {
        mkdirs(*resolved.fs, resolved.localPath);
    }

    fs::File file = resolved.fs->open(resolved.localPath.c_str(), index == 0 ? "w" : "a");

    if (!file)
    {
        loggerInstance->Error("Failed to open file for writing");
        request->_tempObject = new std::string("{\"error\":\"Failed to open file for writing\"}");
        return;
    }
    file.write(data, len);
    file.close();
    yield();

    if (final)
    {
        loggerInstance->Info(std::string("Upload finished: ") + safePath);
    } });