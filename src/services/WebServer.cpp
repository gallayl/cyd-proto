#include "../config.h"

#if ENABLE_WEBSERVER

#include "WebServer.h"

// define the global server instance
AsyncWebServer server(HTTP_PORT);

void initWebServer()
{
    server.reset();

    LoggerInstance->Info(F("Starting WEB server"));

    server.on("/heap", HTTP_GET,
              [](AsyncWebServerRequest *request) { request->send(200, MIME_plainText, String(ESP.getFreeHeap())); });

    server.on("/uploadFiles", HTTP_POST, onPostUploadFiles, uploadFiles);
    server.on("/listFiles", HTTP_GET, listFiles);

    server.serveStatic("/", LittleFS, "/", "max-age=600").setDefaultFile("index.html");

    server.onNotFound(
        [](AsyncWebServerRequest *req)
        {
            LoggerInstance->Info("Not found: " + req->url());
            req->send(404);
        });

    server.begin();

    LoggerInstance->Info(F("Server setup done."));
}

#endif
