#include "Logging.h"
#include "../../ActionRegistry/ActionRegistry.h"
#include "../../utils/CJsonHelper.h"

// instantiate globals
Logger *loggerInstance = new Logger();

static FeatureAction logAction = {.name = "log",
                                  .handler =
                                      [](const std::string & /*command*/)
                                  {
                                      std::string output;
                                      loggerInstance->withEntries([&output](cJSON *entries)
                                                                  { output = cJsonToString(entries); });
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
