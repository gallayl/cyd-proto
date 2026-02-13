#pragma once

#include <ESPAsyncWebServer.h>
#include "../config.h"
#include "./WebServer.h"
#include "../CommandInterpreter/CommandInterpreter.h"
#include "../FeatureRegistry/Features/Logging.h"

AsyncWebSocket *webSocket;

void initWebSockets()
{
    webSocket = new AsyncWebSocket(WEBSOCKETS_URL);

    webSocket->onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
                       {
        if (type == WS_EVT_CONNECT)
        {
            LoggerInstance->Info(String("WS connected: " + client->remoteIP().toString()));
        }
        else if (type == WS_EVT_DISCONNECT)
        {
            LoggerInstance->Info(String("WS left: " + client->remoteIP().toString()));
        }
        else if (type == WS_EVT_DATA)
        {
            String str = String((char *)data).substring(0, len);
            String response = CommandInterpreterInstance->ExecuteCommand(str);
            server->textAll(response);
        } });

    LoggerInstance->AddListener([](String scope, String message)
                                {
        webSocket->textAll(scope + ":" +  message);
    });

    server.addHandler(webSocket);
}