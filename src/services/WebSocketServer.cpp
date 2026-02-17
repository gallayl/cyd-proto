#include "../config.h"

#if ENABLE_WEBSERVER

#include "WebSocketServer.h"
#include "../FeatureRegistry/Features/Logging.h"
#include <string>

#ifdef USE_ESP_IDF

#include <esp_http_server.h>
#include <esp_log.h>
#include <vector>
#include <mutex>
#include <algorithm>
#include "WebServer.h"
#include "../ActionRegistry/ActionRegistry.h"

static const char *WS_TAG = "WebSocket";

static std::vector<int> wsClients;
static std::mutex wsClientsMutex;

static void wsAddClient(int fd)
{
    std::lock_guard<std::mutex> lock(wsClientsMutex);
    if (std::find(wsClients.begin(), wsClients.end(), fd) == wsClients.end())
    {
        wsClients.push_back(fd);
    }
}

static void wsRemoveClient(int fd)
{
    std::lock_guard<std::mutex> lock(wsClientsMutex);
    wsClients.erase(std::remove(wsClients.begin(), wsClients.end(), fd), wsClients.end());
}

void wsOnSessionClose(int sockfd)
{
    wsRemoveClient(sockfd);
}

void wsBroadcast(const std::string &msg)
{
    httpd_handle_t server = getHttpServer();
    if (!server)
        return;

    httpd_ws_frame_t frame;
    memset(&frame, 0, sizeof(frame));
    frame.type = HTTPD_WS_TYPE_TEXT;
    frame.payload = (uint8_t *)msg.c_str();
    frame.len = msg.length();

    std::lock_guard<std::mutex> lock(wsClientsMutex);
    for (int fd : wsClients)
    {
        if (httpd_ws_send_frame_async(server, fd, &frame) != ESP_OK)
        {
            ESP_LOGW(WS_TAG, "Failed to send WS frame to fd %d", fd);
        }
    }
}

static esp_err_t wsHandler(httpd_req_t *req)
{
    if (req->method == HTTP_GET)
    {
        int fd = httpd_req_to_sockfd(req);
        wsAddClient(fd);
        loggerInstance->Info("WS connected: fd=" + std::to_string(fd));
        return ESP_OK;
    }

    httpd_ws_frame_t frame;
    memset(&frame, 0, sizeof(frame));
    frame.type = HTTPD_WS_TYPE_TEXT;

    // First call: get frame length
    esp_err_t ret = httpd_ws_recv_frame(req, &frame, 0);
    if (ret != ESP_OK)
    {
        ESP_LOGE(WS_TAG, "httpd_ws_recv_frame failed (len): %s", esp_err_to_name(ret));
        return ret;
    }

    if (frame.len == 0)
        return ESP_OK;

    uint8_t *buf = static_cast<uint8_t *>(malloc(frame.len + 1));
    if (!buf)
        return ESP_ERR_NO_MEM;

    frame.payload = buf;
    ret = httpd_ws_recv_frame(req, &frame, frame.len);
    if (ret != ESP_OK)
    {
        ESP_LOGE(WS_TAG, "httpd_ws_recv_frame failed (payload): %s", esp_err_to_name(ret));
        free(buf);
        return ret;
    }
    buf[frame.len] = '\0';

    if (frame.type == HTTPD_WS_TYPE_TEXT)
    {
        std::string str(reinterpret_cast<char *>(buf), frame.len);
        std::string response = actionRegistryInstance->execute(str, Transport::WS);

        httpd_ws_frame_t resp;
        memset(&resp, 0, sizeof(resp));
        resp.type = HTTPD_WS_TYPE_TEXT;
        resp.payload = reinterpret_cast<uint8_t *>(const_cast<char *>(response.c_str()));
        resp.len = response.length();
        ret = httpd_ws_send_frame(req, &resp);
    }
    else if (frame.type == HTTPD_WS_TYPE_CLOSE)
    {
        int fd = httpd_req_to_sockfd(req);
        wsRemoveClient(fd);
        loggerInstance->Info("WS closed: fd=" + std::to_string(fd));
    }

    free(buf);
    return ret;
}

void initWebSockets()
{
    httpd_handle_t server = getHttpServer();
    if (!server)
    {
        loggerInstance->Error("Cannot init WebSocket: HTTP server not started");
        return;
    }

    static const httpd_uri_t wsUri = {
        .uri = WEBSOCKETS_URL, .method = HTTP_GET, .handler = wsHandler, .user_ctx = nullptr, .is_websocket = true};
    httpd_register_uri_handler(server, &wsUri);

    loggerInstance->AddListener(
        [](const std::string &scope, const std::string &message)
        {
            std::string buf;
            buf.reserve(scope.length() + 1 + message.length());
            buf += scope;
            buf += ':';
            buf += message;
            wsBroadcast(buf);
        });

    loggerInstance->Info("WebSocket server initialized on " + std::string(WEBSOCKETS_URL));
}

#else // Arduino

#include "./WebServer.h"
#include "../ActionRegistry/ActionRegistry.h"

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

#endif // USE_ESP_IDF

#endif // ENABLE_WEBSERVER
