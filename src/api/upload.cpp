#include "upload.h"
#include "../mime.h"
#include "../fs/VirtualFS.h"

ArRequestHandlerFunction onPostUploadFiles = ([](AsyncWebServerRequest *request) {});

static String sanitizePath(const String &raw)
{
    String path = raw;
    path.replace("\\", "/");
    path.trim();

    // Remove ".." segments to prevent directory traversal
    while (path.indexOf("..") >= 0) {
        path.replace("..", "");
    }

    // Collapse double slashes
    while (path.indexOf("//") >= 0) {
        path.replace("//", "/");
    }

    // Ensure path starts with /
    if (!path.startsWith("/")) {
        path = "/" + path;
    }

    // Remove trailing slash (unless it's the root)
    if (path.length() > 1 && path.endsWith("/")) {
        path.remove(path.length() - 1);
    }

    if (path.length() <= 1) {
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
        if (slash < 0) {
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
    String safePath = sanitizePath(filename);

    if (safePath.isEmpty())
    {
        request->send(400, MIME_json, "{\"error\":\"Invalid file path\"}");
        return;
    }

    ResolvedPath resolved = resolveVirtualPath(safePath);
    if (!resolved.valid || !resolved.fs)
    {
        request->send(400, MIME_json, "{\"error\":\"Path must start with /flash or /sd\"}");
        return;
    }

    if (index == 0) {
        mkdirs(*resolved.fs, resolved.localPath);
    }

    fs::File file = resolved.fs->open(resolved.localPath, index == 0 ? "w" : "a");

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
        LoggerInstance->Info(String(F("Upload finished: ")) + safePath);
        request->send(200, MIME_json, "{\"status\":\"ok\"}");
    } });