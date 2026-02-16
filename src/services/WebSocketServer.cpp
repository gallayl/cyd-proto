#include "../config.h"

#if ENABLE_WEBSERVER

#include "WebSocketServer.h"
#include "./WebServer.h"
#include "../FeatureRegistry/Features/Logging.h"
#include <string>

AsyncWebSocket *webSocket = nullptr;

void initWebSockets()
{
    webSocket = new AsyncWebSocket(WEBSOCKETS_URL);

    webSocket->onEvent(
        [](AsyncWebSocket * /*server*/, AsyncWebSocketClient *client, AwsEventType type, void * /*arg*/, uint8_t *data,
           size_t len)
        {
            if (type == WS_EVT_CONNECT)
            {
                std::string msg = "WS connected: ";
                msg += client->remoteIP().toString().c_str();
                loggerInstance->Info(msg);
            }
            else if (type == WS_EVT_DISCONNECT)
            {
                std::string msg = "WS left: ";
                msg += client->remoteIP().toString().c_str();
                loggerInstance->Info(msg);
            }
            else if (type == WS_EVT_DATA)
            {
                std::string str((const char *)data, len);
                std::string response = actionRegistryInstance->execute(str, Transport::WS);
                client->text(response.c_str());
            }
        });

    loggerInstance->AddListener(
        [](const std::string &scope, const std::string &message)
        {
            if (!webSocket)
            {
                return;
            }
            std::string buf;
            buf.reserve(scope.length() + 1 + message.length());
            buf += scope;
            buf += ':';
            buf += message;
            webSocket->textAll(buf.c_str());
        });

    server.addHandler(webSocket);
}

#endif
