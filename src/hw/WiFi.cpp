#include "WiFi.h"

#ifdef USE_ESP_IDF

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_log.h"

static const char *TAG = "WiFi";

static esp_netif_t *s_sta_netif = nullptr;
static esp_netif_t *s_ap_netif = nullptr;
static EventGroupHandle_t s_wifi_event_group = nullptr;

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_GOT_IP_BIT BIT1

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT)
    {
        switch (event_id)
        {
        case WIFI_EVENT_STA_CONNECTED:
            xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
            break;
        case WIFI_EVENT_STA_DISCONNECTED:
            xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_GOT_IP_BIT);
            break;
        default:
            break;
        }
    }
    else if (event_base == IP_EVENT)
    {
        if (event_id == IP_EVENT_STA_GOT_IP)
        {
            ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
            ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
            xEventGroupSetBits(s_wifi_event_group, WIFI_GOT_IP_BIT);
        }
    }
}

// --- State accessors ---

esp_netif_t *getWifiStaNetif()
{
    return s_sta_netif;
}

esp_netif_t *getWifiApNetif()
{
    return s_ap_netif;
}

bool isWifiStaConnected()
{
    if (s_wifi_event_group == nullptr)
        return false;
    return (xEventGroupGetBits(s_wifi_event_group) & WIFI_GOT_IP_BIT) != 0;
}

// --- WiFi functions ---

bool hasStoredCredentials()
{
    wifi_config_t conf = {};
    esp_wifi_get_config(WIFI_IF_STA, &conf);
    return strlen((char *)conf.sta.ssid) > 0;
}

void startStaMode(const std::string &ssid, const std::string &passphrase)
{
    wifi_mode_t mode;
    esp_wifi_get_mode(&mode);

    if (mode == WIFI_MODE_AP || mode == WIFI_MODE_NULL)
    {
        esp_wifi_set_mode(WIFI_MODE_APSTA);
    }

    wifi_config_t ap_config = {};
    strncpy((char *)ap_config.ap.ssid, ssid.c_str(), sizeof(ap_config.ap.ssid) - 1);
    strncpy((char *)ap_config.ap.password, passphrase.c_str(), sizeof(ap_config.ap.password) - 1);
    ap_config.ap.ssid_len = (uint8_t)ssid.length();
    ap_config.ap.max_connection = 4;
    ap_config.ap.authmode = passphrase.empty() ? WIFI_AUTH_OPEN : WIFI_AUTH_WPA2_PSK;

    esp_wifi_set_config(WIFI_IF_AP, &ap_config);
}

void initWifi()
{
    // Initialize NVS (required for WiFi credential storage)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        nvs_flash_erase();
        nvs_flash_init();
    }

    s_wifi_event_group = xEventGroupCreate();

    // Initialize TCP/IP stack and event loop
    esp_netif_init();
    esp_event_loop_create_default();

    s_sta_netif = esp_netif_create_default_wifi_sta();
    s_ap_netif = esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    // Register event handlers
    esp_event_handler_instance_t wifi_event_instance;
    esp_event_handler_instance_t ip_event_instance;
    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                        &wifi_event_handler, NULL, &wifi_event_instance);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                        &wifi_event_handler, NULL, &ip_event_instance);

    // Start in AP mode with soft AP
    esp_wifi_set_mode(WIFI_MODE_AP);
    startStaMode(STA_SSID, STA_PASSPHRASE);

    if (!hasStoredCredentials())
    {
        loggerInstance->Info("No WiFi credentials saved, starting in AP mode only");
        esp_wifi_start();
        return;
    }

    // Have stored credentials - try connecting
    esp_wifi_set_mode(WIFI_MODE_APSTA);
    esp_wifi_start();
    esp_wifi_connect();

    // Wait for IP with 10-second timeout
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_GOT_IP_BIT, pdFALSE, pdTRUE, pdMS_TO_TICKS(10000));

    if (bits & WIFI_GOT_IP_BIT)
    {
        loggerInstance->Info("Connected to access point");
    }
    else
    {
        loggerInstance->Error("Failed to connect to access point");
    }
}

#endif // USE_ESP_IDF
