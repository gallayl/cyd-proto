#pragma once

#include "../config.h"
#include <FS.h>
#include <LittleFS.h>

#if ENABLE_SD_CARD
#include <SD.h>
#endif

struct ResolvedPath
{
    fs::FS *fs;
    String localPath;
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

inline ResolvedPath resolveVirtualPath(const String &virtualPath)
{
    if (virtualPath.startsWith("/flash"))
    {
        String local = virtualPath.substring(6); // strip "/flash"
        if (local.isEmpty())
            local = "/";
        return {&LittleFS, local, true};
    }

#if ENABLE_SD_CARD
    if (virtualPath.startsWith("/sd"))
    {
        String local = virtualPath.substring(3); // strip "/sd"
        if (local.isEmpty())
            local = "/";
        return {&SD, local, true};
    }
#endif

    return {nullptr, "", false};
}

inline String getVirtualPrefix(fs::FS *fs)
{
    if (fs == &LittleFS)
        return "/flash";

#if ENABLE_SD_CARD
    if (fs == &SD)
        return "/sd";
#endif

    return "";
}
