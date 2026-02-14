#include "LittleFsManagement.h"
#include "../../../ActionRegistry/ActionRegistry.h"
#include "../../../CommandInterpreter/CommandParser.h"
#include "../../../fs/VirtualFS.h"

FeatureAction formatAction = {
    .name = "format",
    .type = "POST",
    .handler = [](const String &command)
    {
        LittleFS.end();
        LittleFS.format();
        if (!LittleFS.begin()) {
            return String("{\"error\": \"Mount after format failed\"}");
        }
        return String("{\"event\": \"format\"}");
    },
    .transports = {.cli = true, .rest = false, .ws = true, .scripting = true}};

FeatureAction listFilesAction = {
    .name = "list",
    .handler = [](const String &command)
    {
        String path = CommandParser::GetCommandParameter(command, 1);

        JsonDocument response;
        if (path.isEmpty())
        {
            response = getFileList(LittleFS, "/");
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

        String output;
        serializeJson(response, output);
        return output;
    },
    .transports = {.cli = true, .rest = false, .ws = true, .scripting = true}};

Feature *LittleFsFeature = new Feature("LittleFsFeatures", []()
                                       {
    // Register format even if setup fails, so the command is available to fix the issue
    ActionRegistryInstance->RegisterAction(&formatAction);

    if (!LittleFS.begin())
    {
        LoggerInstance->Error(F("LittleFS not available"));
        return FeatureState::ERROR;
    }

    ActionRegistryInstance->RegisterAction(&listFilesAction);
    return FeatureState::RUNNING; }, []() {

                                       });
