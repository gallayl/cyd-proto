#include "../config.h"

#if ENABLE_WEBSERVER

#include "WebServer.h"
#include "../mime.h"
#include "../utils/System.h"
#include <string>

#ifdef USE_ESP_IDF

// ESP-IDF web server (esp_http_server) will be implemented in Phase 5.
// For now, provide a stub so the project compiles.
void initWebServer()
{
    loggerInstance->Info("Web server init (ESP-IDF): not yet implemented (Phase 5)");
}

#else // Arduino

#include "../api/upload.h"
#include "../api/list.h"

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

#endif // USE_ESP_IDF

#endif // ENABLE_WEBSERVER
