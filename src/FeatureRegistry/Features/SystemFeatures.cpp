#include "SystemFeatures.h"

// define commands
CustomCommand *resetCommand = new CustomCommand("restart", [](const String &command)
                                                {
    delay(100);
    ESP.restart();
    return String("{\"event\": \"restart\"}"); });

CustomCommand *getRegisteredFeatures = new CustomCommand("getRegisteredFeatures", [](const String &command)
                                                         {
    String output;
    serializeJson(registeredFeatures, output);
    return output; });

#if ENABLE_WEBSERVER
// handlers
ArRequestHandlerFunction getFeaturesAction = [](AsyncWebServerRequest *request)
{
    String output;
    serializeJson(registeredFeatures, output);
    request->send(200, MIME_json, output);
};

ArRequestHandlerFunction reset = [](AsyncWebServerRequest *request)
{
    request->send(200, MIME_json, "{\"event\": \"restart\"}");
    delay(100);
    ESP.restart();
};

ArRequestHandlerFunction getInfoAction = [](AsyncWebServerRequest *request)
{
    AsyncJsonResponse *resp = new AsyncJsonResponse();
    JsonDocument info = getInfo();
    resp->setCode(200);
    resp->getRoot().set(info);
    resp->setLength();
    request->send(resp);
};
#endif

// feature object
Feature *SystemFeatures = new Feature("SystemFeatures", []()
                                      {
#if ENABLE_WIFI
    CommandInterpreterInstance->RegisterCommand(wifiCommand);
#endif
    CommandInterpreterInstance->RegisterCommand(resetCommand);
    CommandInterpreterInstance->RegisterCommand(getRegisteredFeatures);
    CommandInterpreterInstance->RegisterCommand(infoCustomCommand);

#if ENABLE_WEBSERVER
    server.on("/features", HTTP_GET, getFeaturesAction);
    server.on("/restart", HTTP_POST, reset);
    server.on("/info", HTTP_GET, getInfoAction);
#endif

    // Init and register low level features and commands
    initRgbLed();
    initLightSensor();
    CommandInterpreterInstance->RegisterCommand(rgbLedCustomCommand);
    CommandInterpreterInstance->RegisterCommand(getLightSensorValueCommand);
    CommandInterpreterInstance->RegisterCommand(getHallSensorValueCommand);

    return FeatureState::RUNNING; }, []() {});
