#include <Wire.h>
#include "./CommandInterpreter/CommandInterpreter.h"
#include "./services/WebServer.h"
#include "./services/WebSocketServer.h"
#include "./hw/WiFi.h"
#include "./hw/Screen.h"
#include "./FeatureRegistry/FeatureRegistry.h"

int16_t throttleValue = 0;
int16_t steerValue = 0;

void setup()
{
    Serial.begin(115200);

    Serial.println("Starting up Sticky...");

    initScreen();
    initWifi();
    initWebServer();
    initWebSockets();

    FeatureRegistryInstance->SetupFeatures();

    lv_obj_t *label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "Hello CYD!");
    lv_obj_center(label);
}

void loop()
{
    FeatureRegistryInstance->LoopFeatures();
}
