#include "Logging.h"

// instantiate globals
Logger *LoggerInstance = new Logger();

CustomCommand *showLogCustomCommand = new CustomCommand("showLog", [](String command)
                                                        {
    char buffer[LOG_BUFFER_LENGTH];
    JsonDocument response = LoggerInstance->getEntries();
    serializeJson(response, buffer);
    return String(buffer);
});

ArRequestHandlerFunction showLogRequestHandler = [](AsyncWebServerRequest *request)
{
    AsyncJsonResponse *resp = new AsyncJsonResponse();
    JsonDocument entries = LoggerInstance->getEntries();
    resp->setCode(200);
    resp->getRoot().set(entries);
    resp->setLength();
    request->send(resp);
};

Feature *LoggingFeature = new Feature("Logging", []()
                                      {
    CommandInterpreterInstance->RegisterCommand(*showLogCustomCommand);
    server.on("/log", HTTP_GET, showLogRequestHandler);
    return FeatureState::RUNNING;
}, []() {});
