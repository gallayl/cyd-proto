#include "Logging.h"
#include "../../ActionRegistry/ActionRegistry.h"

// instantiate globals
Logger *loggerInstance = new Logger();

static FeatureAction logAction = {.name = "log",
                                  .handler =
                                      [](const String & /*command*/)
                                  {
                                      const JsonDocument &entries = loggerInstance->getEntries();
                                      String output;
                                      serializeJson(entries, output);
                                      return output;
                                  },
                                  .transports = {.cli = true, .rest = true, .ws = true, .scripting = true}};

Feature *loggingFeature = new Feature(
    "Logging",
    []()
    {
        actionRegistryInstance->registerAction(&logAction);
        return FeatureState::RUNNING;
    },
    []() {});
