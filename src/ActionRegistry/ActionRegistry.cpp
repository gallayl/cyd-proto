#include "ActionRegistry.h"

ActionRegistry *actionRegistryInstance = new ActionRegistry();

#if ENABLE_WEBSERVER

#include "../mime.h"

#ifdef USE_ESP_IDF

#include "../services/WebServer.h"

static esp_err_t actionRestHandler(httpd_req_t *req)
{
    FeatureAction *action = static_cast<FeatureAction *>(req->user_ctx);
    std::string result = action->handler(action->name);
    httpd_resp_set_type(req, MIME_JSON);
    return httpd_resp_send(req, result.c_str(), HTTPD_RESP_USE_STRLEN);
}

void ActionRegistry::wireRestEndpoints()
{
    httpd_handle_t server = getHttpServer();
    if (!server)
        return;

    for (uint8_t i = 0; i < _registeredActionsCount; i++)
    {
        FeatureAction *action = _actions[i];
        if (!action->transports.rest)
            continue;

        std::string pathStr = "/" + action->name;
        char *uri = strdup(pathStr.c_str());

        httpd_method_t method = HTTP_GET;
        if (action->type == "POST")
            method = HTTP_POST;
        else if (action->type == "PUT")
            method = HTTP_PUT;
        else if (action->type == "DELETE")
            method = HTTP_DELETE;

        const httpd_uri_t uriHandler = {.uri = uri, .method = method, .handler = actionRestHandler, .user_ctx = action};
        httpd_register_uri_handler(server, &uriHandler);
    }
}

#else // Arduino

#include <ESPAsyncWebServer.h>

extern AsyncWebServer server;

void ActionRegistry::wireRestEndpoints()
{
    for (uint8_t i = 0; i < _registeredActionsCount; i++)
    {
        FeatureAction *action = _actions[i];
        if (!action->transports.rest)
        {
            continue;
        }

        std::string path = "/" + action->name;

        WebRequestMethodComposite method = HTTP_GET;
        if (action->type == "POST")
        {
            method = HTTP_POST;
        }
        else if (action->type == "PUT")
        {
            method = HTTP_PUT;
        }
        else if (action->type == "DELETE")
        {
            method = HTTP_DELETE;
        }

        server.on(path.c_str(), method,
                  [action](AsyncWebServerRequest *request)
                  {
                      std::string result = action->handler(action->name);
                      request->send(200, MIME_JSON, result.c_str());
                  });
    }
}

#endif // USE_ESP_IDF

#endif // ENABLE_WEBSERVER
