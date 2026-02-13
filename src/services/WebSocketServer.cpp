#include "WebSocketServer.h"

// global pointer definition
AsyncWebSocket *webSocket = nullptr;

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
            String str;
            str.concat((const char *)data, len);
            String response = CommandInterpreterInstance->ExecuteCommand(str);
            client->text(response);
        } });

    LoggerInstance->AddListener([](const String &scope, const String &message)
                                {
        webSocket->textAll(scope + ":" +  message);
    });

    server.addHandler(webSocket);
}
