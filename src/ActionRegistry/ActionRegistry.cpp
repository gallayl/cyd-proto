#include "ActionRegistry.h"

ActionRegistry *actionRegistryInstance = new ActionRegistry();

#if ENABLE_WEBSERVER

#include <ESPAsyncWebServer.h>
#include "../mime.h"

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

#endif
