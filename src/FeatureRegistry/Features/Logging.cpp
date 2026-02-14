#include "Logging.h"
#include "../../ActionRegistry/ActionRegistry.h"

// instantiate globals
Logger *LoggerInstance = new Logger();

FeatureAction logAction = {
    .name = "log",
    .handler = [](const String &command)
    {
        const JsonDocument &entries = LoggerInstance->getEntries();
        String output;
        serializeJson(entries, output);
        return output;
    },
    .transports = {.cli = true, .rest = true, .ws = true, .scripting = true}};

Feature *LoggingFeature = new Feature("Logging", []()
                                      {
    ActionRegistryInstance->RegisterAction(&logAction);
    return FeatureState::RUNNING; }, []() {});
