#include "LittleFsManagement.h"
#include "../../../ActionRegistry/ActionRegistry.h"

FeatureAction formatAction = {
    .name = "format",
    .type = "POST",
    .handler = [](const String &command)
    {
        LittleFS.format();
        return String("{\"event\": \"format\"}");
    },
    .transports = {.cli = true, .rest = false, .ws = true, .scripting = true}};

FeatureAction listFilesAction = {
    .name = "list",
    .handler = [](const String &command)
    {
        JsonDocument response = getFileList();
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
