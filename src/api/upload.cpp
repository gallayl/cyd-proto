#include "upload.h"
#include "../mime.h"
#include "../fs/VirtualFS.h"
#include <Arduino.h>

ArRequestHandlerFunction onPostUploadFiles = ([](AsyncWebServerRequest *request) {
    // Check if there was an error during upload (stored in _tempObject)
    if (request->_tempObject != nullptr) {
        String* errorMsg = (String*)request->_tempObject;
        request->send(400, MIME_json, *errorMsg);
        delete errorMsg;
        request->_tempObject = nullptr;
        return;
    }
    request->send(200, MIME_json, "{\"status\":\"ok\"}");
});

static String sanitizePath(const String &raw)
{
    String path = raw;
    path.replace("\\", "/");
    path.trim();

    // Remove ".." segments to prevent directory traversal
    while (path.indexOf("..") >= 0)
    {
        path.replace("..", "");
    }

    // Collapse double slashes
    while (path.indexOf("//") >= 0)
    {
        path.replace("//", "/");
    }

    // Ensure path starts with /
    if (!path.startsWith("/"))
    {
        path = "/" + path;
    }

    // Remove trailing slash (unless it's the root)
    if (path.length() > 1 && path.endsWith("/"))
    {
        path.remove(path.length() - 1);
    }

    if (path.length() <= 1)
    {
        return "";
    }

    return path;
}

static bool mkdirs(fs::FS &fs, const String &filePath)
{
    int idx = 1; // skip leading '/'
    while (true)
    {
        int slash = filePath.indexOf('/', idx);
        if (slash < 0)
        {
            break;
        }
        String dir = filePath.substring(0, slash);
        fs.mkdir(dir.c_str());
        idx = slash + 1;
    }
    return true;
}

ArUploadHandlerFunction uploadFiles = ([](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
                                       {
    // If there's already an error, skip further processing
    if (request->_tempObject != nullptr) {
        return;
    }

    String targetPath = request->arg("path");

    if (targetPath.isEmpty()) {
        String basePath = request->arg("basePath");
        if (!basePath.isEmpty()) {
            if (!basePath.endsWith("/")) {
                basePath += "/";
            }
            targetPath = basePath + filename;
        } else {
            targetPath = filename;
        }
    }

    String safePath = sanitizePath(targetPath);

    if (safePath.isEmpty())
    {
        request->_tempObject = new String("{\"error\":\"Invalid file path\"}");
        return;
    }

    ResolvedPath resolved = resolveVirtualPath(safePath);
    if (!resolved.valid || !resolved.fs)
    {
        request->_tempObject = new String("{\"error\":\"Path must start with /flash or /sd\"}");
        return;
    }

    if (index == 0) {
        mkdirs(*resolved.fs, resolved.localPath);
    }

    fs::File file = resolved.fs->open(resolved.localPath, index == 0 ? "w" : "a");

    if (!file)
    {
        LoggerInstance->Error(F("Failed to open file for writing"));
        request->_tempObject = new String("{\"error\":\"Failed to open file for writing\"}");
        return;
    }
    file.write(data, len);
    file.close();
    yield();

    if (final)
    {
        LoggerInstance->Info(String(F("Upload finished: ")) + safePath);
    } });