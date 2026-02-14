#include <Wire.h>
#include "./config.h"
#include "./CommandInterpreter/CommandInterpreter.h"

#if ENABLE_WEBSERVER
#include "./services/WebServer.h"
#include "./services/WebSocketServer.h"
#endif

#if ENABLE_WIFI
#include "./hw/WiFi.h"
#endif

#if ENABLE_SCREEN
#include "./hw/Screen.h"
#endif

#include "./FeatureRegistry/FeatureRegistry.h"

void setup()
{
    Serial.begin(115200);

    Serial.println("Starting up Sticky...");

#if ENABLE_SCREEN
    initScreen();
#endif

#if ENABLE_WIFI
    initWifi();
#endif

#if ENABLE_WEBSERVER
    initWebServer();
    initWebSockets();
#endif

    FeatureRegistryInstance->Init();
    FeatureRegistryInstance->SetupFeatures();
}

void loop()
{
    FeatureRegistryInstance->LoopFeatures();
    delay(1);
}
