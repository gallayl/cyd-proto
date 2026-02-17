#pragma once

#include "../config.h"
#include "../utils/StringUtil.h"
#include "LittleFsInit.h"
#include <string>

#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

#if ENABLE_SD_CARD
#include "../FeatureRegistry/Features/SdCard/SdCardFeature.h"
#endif

struct ResolvedPath
{
    std::string realPath;
    bool valid;
};

inline bool isSdMounted()
{
#if ENABLE_SD_CARD
    return isSdCardMounted();
#else
    return false;
#endif
}

inline ResolvedPath resolveVirtualPath(const std::string &virtualPath)
{
    if (StringUtil::startsWith(virtualPath, "/flash"))
    {
        std::string local = virtualPath.substr(6); // strip "/flash"
        if (local.empty())
        {
            local = "/";
        }
        return {std::string(LITTLEFS_MOUNT_POINT) + local, true};
    }

#if ENABLE_SD_CARD
    if (StringUtil::startsWith(virtualPath, "/sd"))
    {
        std::string local = virtualPath.substr(3); // strip "/sd"
        if (local.empty())
        {
            local = "/";
        }
        return {std::string(SD_MOUNT_POINT) + local, true};
    }
#endif

    return {"", false};
}

inline std::string toVirtualPath(const std::string &realPath)
{
    if (StringUtil::startsWith(realPath, LITTLEFS_MOUNT_POINT))
    {
        return "/flash" + realPath.substr(strlen(LITTLEFS_MOUNT_POINT));
    }

#if ENABLE_SD_CARD
    if (StringUtil::startsWith(realPath, SD_MOUNT_POINT))
    {
        return "/sd" + realPath.substr(strlen(SD_MOUNT_POINT));
    }
#endif

    return realPath;
}

inline std::string resolveToLittleFsPath(const std::string &localPath)
{
    return std::string(LITTLEFS_MOUNT_POINT) + localPath;
}

// --- Convenience helpers for standard C file I/O ---

inline bool vfsExists(const std::string &path)
{
    struct stat st;
    return stat(path.c_str(), &st) == 0;
}

inline bool vfsIsDirectory(const std::string &path)
{
    struct stat st;
    if (stat(path.c_str(), &st) != 0)
    {
        return false;
    }
    return S_ISDIR(st.st_mode);
}

inline std::string vfsReadFileAsString(const std::string &path)
{
    FILE *f = fopen(path.c_str(), "r");
    if (!f)
    {
        return "";
    }
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (size <= 0)
    {
        fclose(f);
        return "";
    }
    std::string content(size, '\0');
    size_t bytesRead = fread(&content[0], 1, size, f);
    content.resize(bytesRead);
    fclose(f);
    return content;
}

inline bool vfsMkdirs(const std::string &filePath)
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
        mkdir(dir.c_str(), 0775);
        idx = slash + 1;
    }
    return true;
}
