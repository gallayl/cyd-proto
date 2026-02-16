#include "../config.h"

#if ENABLE_WEBSERVER

#include "WebSocketServer.h"
#include "./WebServer.h"
#include "../FeatureRegistry/Features/Logging.h"

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
                String msg(F("WS connected: "));
                msg += client->remoteIP().toString();
                loggerInstance->Info(msg);
            }
            else if (type == WS_EVT_DISCONNECT)
            {
                String msg(F("WS left: "));
                msg += client->remoteIP().toString();
                loggerInstance->Info(msg);
            }
            else if (type == WS_EVT_DATA)
            {
                String str;
                str.concat((const char *)data, len);
                String response = actionRegistryInstance->execute(str, Transport::WS);
                client->text(response);
            }
        });

    loggerInstance->AddListener(
        [](const String &scope, const String &message)
        {
            if (!webSocket)
            {
                return;
            }
            String buf;
            buf.reserve(scope.length() + 1 + message.length());
            buf += scope;
            buf += ':';
            buf += message;
            webSocket->textAll(buf);
        });

    server.addHandler(webSocket);
}

#endif
