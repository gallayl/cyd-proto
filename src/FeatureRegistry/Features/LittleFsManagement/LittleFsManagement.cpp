#include "LittleFsManagement.h"
#include "../../../ActionRegistry/ActionRegistry.h"
#include "../../../CommandInterpreter/CommandParser.h"
#include "../../../fs/VirtualFS.h"
#include "../../../fs/LittleFsInit.h"
#include <string>

#include "cJSON.h"
#include "../../../utils/CJsonHelper.h"

static FeatureAction formatAction = {.name = "format",
                                     .type = "POST",
                                     .handler =
                                         [](const std::string & /*command*/)
                                     {
                                         deinitLittleFs();
                                         formatLittleFs();
                                         if (!initLittleFs())
                                         {
                                             return std::string("{\"error\": \"Mount after format failed\"}");
                                         }
                                         return std::string("{\"event\": \"format\"}");
                                     },
                                     .transports = {.cli = true, .rest = false, .ws = true, .scripting = true}};

static FeatureAction listFilesAction = {.name = "list",
                                        .handler =
                                            [](const std::string &command)
                                        {
                                            std::string path = CommandParser::getCommandParameter(command, 1);
                                            if (path.empty())
                                            {
                                                path = "/";
                                            }

                                            cJSON *response = nullptr;

                                            if (path == "/")
                                            {
                                                response = cJSON_CreateArray();

                                                cJSON *flash = cJSON_CreateObject();
                                                cJSON_AddStringToObject(flash, "name", "flash");
                                                cJSON_AddNumberToObject(flash, "size", 0);
                                                cJSON_AddBoolToObject(flash, "isDir", true);
                                                cJSON_AddNumberToObject(flash, "lastWrite", 0);
                                                cJSON_AddItemToArray(response, flash);

#if ENABLE_SD_CARD
                                                if (isSdMounted())
                                                {
                                                    cJSON *sd = cJSON_CreateObject();
                                                    cJSON_AddStringToObject(sd, "name", "sd");
                                                    cJSON_AddNumberToObject(sd, "size", 0);
                                                    cJSON_AddBoolToObject(sd, "isDir", true);
                                                    cJSON_AddNumberToObject(sd, "lastWrite", 0);
                                                    cJSON_AddItemToArray(response, sd);
                                                }
#endif
                                            }
                                            else
                                            {
                                                ResolvedPath resolved = resolveVirtualPath(path);
                                                if (resolved.valid)
                                                {
                                                    response = getFileList(resolved.realPath);
                                                }
                                                else
                                                {
                                                    response = getFileList(resolveToLittleFsPath(path));
                                                }
                                            }

                                            std::string output = cJsonToString(response);
                                            cJSON_Delete(response);
                                            return output;
                                        },
                                        .transports = {.cli = true, .rest = false, .ws = true, .scripting = true}};

Feature *LittleFsFeature = new Feature("LittleFsFeatures", []()
                                       {
    // Register format even if setup fails, so the command is available to fix the issue
    actionRegistryInstance->registerAction(&formatAction);

    if (!initLittleFs())
    {
        loggerInstance->Error("LittleFS not available");
        return FeatureState::ERROR;
    }

    actionRegistryInstance->registerAction(&listFilesAction);
    return FeatureState::RUNNING; }, []() {

                                       });
