#include "Logging.h"

// instantiate globals
Logger *LoggerInstance = new Logger();

CustomCommand *showLogCustomCommand = new CustomCommand("showLog", [](String command)
                                                        {
    JsonDocument response = LoggerInstance->getEntries();
    String output;
    serializeJson(response, output);
    return output; });

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
    return FeatureState::RUNNING; }, []() {});
