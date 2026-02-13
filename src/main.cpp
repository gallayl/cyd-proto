#include <Wire.h>
#include "./CommandInterpreter/CommandInterpreter.h"
#include "./services/WebServer.h"
#include "./services/WebSocketServer.h"
#include "./hw/WiFi.h"
#include "./hw/Screen.h"
#include "./FeatureRegistry/FeatureRegistry.h"

void setup()
{
    Serial.begin(115200);

    Serial.println("Starting up Sticky...");

    initScreen();
    initWifi();
    initWebServer();
    initWebSockets();

    FeatureRegistryInstance->Init();
    FeatureRegistryInstance->SetupFeatures();
}

void loop()
{
    FeatureRegistryInstance->LoopFeatures();
}
