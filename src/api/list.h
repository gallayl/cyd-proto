#pragma once

#include "../config.h"
#include <string>
#include "cJSON.h"

cJSON *getFileList();
cJSON *getFileList(const char *path);
cJSON *getFileList(const std::string &realPath);
