#include "wifiCommand.h"
#include "../../../config.h"

CustomCommand *wifiCommand = new CustomCommand("wifi", [](String command)
                                               {
    String operation = CommandParser::GetCommandParameter(command, 1);
    if (!operation.compareTo("connect"))
    {
        String ssid = CommandParser::GetCommandParameter(command, 2);
        String password = CommandParser::GetCommandParameter(command, 3);
        if (ssid.length() < 3 || password.length() < 5)
        {
            return String("{\"error\": \"ssid or password too short\"}");
        }
        WiFi.disconnect(true, false);
        WiFi.persistent(true);
        WiFi.mode(WIFI_AP);
        WiFi.begin(ssid.c_str(), password.c_str());
        return String("{\"event\": \"connecting\"}");
    }
    if (!operation.compareTo("list"))
    {
        JsonDocument response = JsonDocument().as<JsonArray>();

        int n = WiFi.scanNetworks();

        for (int i = 0; i < n; ++i)
        {
            JsonObject element = response.as<JsonArray>().add<JsonObject>();
            element["ssid"] = WiFi.SSID(i);
            element["rssi"] = WiFi.RSSI(i);
            element["rssiText"] = getSignalStrength(WiFi.RSSI(i));
            element["encryption"] = getEncryptionType(WiFi.encryptionType(i));
            response.add(element);
        }
        char buffer[JSON_BUFFER_SIZE];
        serializeJson(response, buffer);
        return String(buffer);
    }
    if (!operation.compareTo("startSTA"))
    {
        String ssid = CommandParser::GetCommandParameter(command, 2);
        String passpharse = CommandParser::GetCommandParameter(command, 3);
        if (ssid.length() < 3 || passpharse.length() < 5)
        {
            return String("{\"error\": \"ssid or passpharse too short\"}");
        }
        startStaMode(ssid, passpharse);
        return String("{\"event\": \"starting STA\"}");
    }

    if (!operation.compareTo("stopSTA"))
    {
        bool success = WiFi.softAPdisconnect(true);
        WiFi.mode(WIFI_AP);
        WiFi.begin();
        return String("{\"event\": \"stopSTA\", \"success\": " + String(success) + "}");
    }

    if (!operation.compareTo("info"))
    {
        JsonDocument response;
        if (WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA)
        {
            JsonObject ap = response["ap"].to<JsonObject>();
            ap["ipAddress"] = WiFi.localIP().toString();
            ap["macAddress"] = WiFi.macAddress();
            ap["ssid"] = WiFi.SSID();
        }

        if (WiFi.getMode() == WIFI_STA || WiFi.getMode() == WIFI_AP_STA)
        {
            JsonObject sta = response["sta"].to<JsonObject>();
            sta["ipAddress"] = WiFi.softAPIP().toString();
            sta["macAddress"] = WiFi.softAPmacAddress();
        }

        int32_t rssi = WiFi.RSSI();
        response["wifiStrengh"] = getSignalStrength(rssi);
        response["wifiRssiDb"] = rssi;

        char buffer[JSON_BUFFER_SIZE];
        serializeJson(response, buffer);
        return String(buffer);
    }

    if (!operation.compareTo("restart"))
    {
        WiFi.disconnect(true, false);
        WiFi.begin();
        return String("{\"event\": \"disconnecting\"}");
    }
    return String("{\"event\": \"Unknown WiFi operation command. The available commands are: info, list, connect <ssid> <password>, startSTA<ssid, password>, stopSTA\"}");
});