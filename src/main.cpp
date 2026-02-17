#include "./config.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

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

static const char *TAG = "main";

static void loopTask(void *arg)
{
    while (1)
    {
        featureRegistryInstance->loopFeatures();
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "Starting up Sticky...");

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

    featureRegistryInstance->init();
    featureRegistryInstance->setupFeatures();
    featureRegistryInstance->startFeatureTasks();

    xTaskCreate(loopTask, "main_loop", 4096, NULL, 1, NULL);
}
