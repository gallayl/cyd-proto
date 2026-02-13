#include "LittleFsManagement.h"

Feature *LittleFsFeature = new Feature("LittleFsFeatures", []()
                                       {
    // Register, even if the feature setup fails, so that the command is available to fix the issue (e.g. by formatting the filesystem)
    CommandInterpreterInstance->RegisterCommand(formatCustomCommand);

    if (!LittleFS.begin())
    {
        LoggerInstance->Error(F("LittleFS not available"));
        return FeatureState::ERROR;
    }

    CommandInterpreterInstance->RegisterCommand(showFileListCustomCommand);
    return FeatureState::RUNNING; }, []() {

                                       });
