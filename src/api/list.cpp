#include "list.h"
#include "../fs/VirtualFS.h"
#include "../mime.h"
#include <string>

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
