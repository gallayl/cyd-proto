#include "LittleFsManagement.h"
#include "../../../ActionRegistry/ActionRegistry.h"
#include "../../../CommandInterpreter/CommandParser.h"
#include "../../../fs/VirtualFS.h"
#include <string>

static FeatureAction formatAction = {.name = "format",
                                     .type = "POST",
                                     .handler =
                                         [](const std::string & /*command*/)
                                     {
                                         LittleFS.end();
                                         LittleFS.format();
                                         if (!LittleFS.begin())
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

                                            JsonDocument response;
                                            JsonArray fileList = response.to<JsonArray>();

                                            if (path == "/")
                                            {
                                                JsonObject flash = fileList.add<JsonObject>();
                                                flash["name"] = "flash";
                                                flash["size"] = 0;
                                                flash["isDir"] = true;
                                                flash["lastWrite"] = 0;

#if ENABLE_SD_CARD
                                                if (isSdMounted())
                                                {
                                                    JsonObject sd = fileList.add<JsonObject>();
                                                    sd["name"] = "sd";
                                                    sd["size"] = 0;
                                                    sd["isDir"] = true;
                                                    sd["lastWrite"] = 0;
                                                }
#endif
                                            }
                                            else
                                            {
                                                ResolvedPath resolved = resolveVirtualPath(path);
                                                if (resolved.valid && resolved.fs)
                                                {
                                                    response = getFileList(*resolved.fs, resolved.localPath.c_str());
                                                }
                                                else
                                                {
                                                    response = getFileList(LittleFS, path.c_str());
                                                }
                                            }

                                            std::string output;
                                            serializeJson(response, output);
                                            return output;
                                        },
                                        .transports = {.cli = true, .rest = false, .ws = true, .scripting = true}};

Feature *LittleFsFeature = new Feature("LittleFsFeatures", []()
                                       {
    // Register format even if setup fails, so the command is available to fix the issue
    actionRegistryInstance->registerAction(&formatAction);

    if (!LittleFS.begin())
    {
        loggerInstance->Error("LittleFS not available");
        return FeatureState::ERROR;
    }

    actionRegistryInstance->registerAction(&listFilesAction);
    return FeatureState::RUNNING; }, []() {

                                       });
