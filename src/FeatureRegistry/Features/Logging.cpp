#include "Logging.h"
#include "../../ActionRegistry/ActionRegistry.h"
#ifdef USE_ESP_IDF
#include "../../utils/CJsonHelper.h"
#endif

// instantiate globals
Logger *loggerInstance = new Logger();

static FeatureAction logAction = {.name = "log",
                                  .handler =
                                      [](const std::string & /*command*/)
                                  {
#ifdef USE_ESP_IDF
                                      std::string output;
                                      loggerInstance->withEntries([&output](cJSON *entries)
                                                                  { output = cJsonToString(entries); });
                                      return output;
#else
                                      const JsonDocument &entries = loggerInstance->getEntries();
                                      std::string output;
                                      serializeJson(entries, output);
                                      return output;
#endif
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
