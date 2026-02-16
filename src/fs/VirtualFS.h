#pragma once

#include "../config.h"
#include <FS.h>
#include <LittleFS.h>
#include <string>
#include "../utils/StringUtil.h"

#if ENABLE_SD_CARD
#include <SD.h>
#endif

struct ResolvedPath
{
    fs::FS *fs;
    std::string localPath;
    bool valid;
};

inline bool isSdMounted()
{
#if ENABLE_SD_CARD
    return SD.cardType() != CARD_NONE;
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
        return {&LittleFS, local, true};
    }

#if ENABLE_SD_CARD
    if (StringUtil::startsWith(virtualPath, "/sd"))
    {
        std::string local = virtualPath.substr(3); // strip "/sd"
        if (local.empty())
        {
            local = "/";
        }
        return {&SD, local, true};
    }
#endif

    return {nullptr, "", false};
}

// Compatibility overload for code not yet migrated from Arduino String
inline ResolvedPath resolveVirtualPath(const String &virtualPath)
{
    return resolveVirtualPath(std::string(virtualPath.c_str()));
}

inline std::string getVirtualPrefix(fs::FS *fs)
{
    if (fs == &LittleFS)
    {
        return "/flash";
    }

#if ENABLE_SD_CARD
    if (fs == &SD)
    {
        return "/sd";
    }
#endif

    return "";
}
