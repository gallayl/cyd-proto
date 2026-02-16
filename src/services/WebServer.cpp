#include "../config.h"

#if ENABLE_WEBSERVER

#include "WebServer.h"
#include "../mime.h"
#include "../api/upload.h"
#include "../api/list.h"
#include "../utils/System.h"
#include <string>

// define the global server instance
AsyncWebServer server(HTTP_PORT);

void initWebServer()
{
    server.reset();

    loggerInstance->Info("Starting WEB server");

    server.on("/heap", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(200, MIME_PLAIN_TEXT, std::to_string(getFreeHeap()).c_str()); });

    server.on("/uploadFiles", HTTP_POST, onPostUploadFiles, uploadFiles);
    server.on("/listFiles", HTTP_GET, listFiles);

    server.serveStatic("/", LittleFS, "/", "max-age=600").setDefaultFile("index.html");

    server.onNotFound(
        [](AsyncWebServerRequest *req)
        {
            loggerInstance->Info(std::string("Not found: ") + req->url().c_str());
            req->send(404);
        });

    server.begin();

    loggerInstance->Info("Server setup done.");
}

#endif
