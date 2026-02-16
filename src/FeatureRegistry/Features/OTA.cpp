#include "../../config.h"

#if ENABLE_OTA

#include "OTA.h"
#include "../../services/WebSocketServer.h" // ensure webSocket is declared (fixed relative path)
#include "../../mime.h"
#include "./Logging.h"
#include "../../services/WebServer.h"

// redundant extern in this translation unit to satisfy IntelliSense

ArRequestHandlerFunction getUpdateForm = ([](AsyncWebServerRequest *request)
                                          { request->send(200, MIME_HTML, F("<form method='POST' action='/update' accept='application/octet-stream' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>")); });

ArRequestHandlerFunction getRedirectPage = ([](AsyncWebServerRequest *request)
                                            {
                                                AsyncWebServerResponse *response = request->beginResponse(200, MIME_HTML, "<html><head><meta http-equiv=\"refresh\" content=\"30\"></head><body>Update done, page will be refreshed.</body></html>");
                                                response->addHeader("Refresh", REFRESH_TIMEOUT_AFTER_UPDATE);
                                                request->send(response); });

ArRequestHandlerFunction onPostUpdate = ([](AsyncWebServerRequest *request)
                                         {
                                             boolean shouldReboot = !Update.hasError();
                                             AsyncWebServerResponse *response = request->beginResponse(200, MIME_PLAIN_TEXT, shouldReboot ? "OK" : "FAIL");
                                             response->addHeader("Connection", "close");
                                             request->send(response); });

ArUploadHandlerFunction onUploadUpdate = ([](AsyncWebServerRequest *request, const String&  /*filename*/, size_t index, uint8_t *data, size_t len, bool final)
                                          {
                                              // ESP32 Update library does not support runAsync
                                              if (Update.hasError())
                                              {
                                                  Update.printError(Serial);
                                                  request->send(500, MIME_PLAIN_TEXT, "Update error");
                                                  return;
                                              }
                                              if (!index)
                                              {
                                                  loggerInstance->Info(F("Starting OTA update..."));
                                                  if (webSocket) {
                                                      webSocket->textAll(R"({"type":"otaUpdateStarted"})");
}
                                                  if (!Update.begin(request->contentLength(), U_FLASH))
                                                  {
                                                      Update.printError(Serial);
                                                  }
                                              }
                                              if (!Update.hasError())
                                              {
                                                  if (Update.write(data, len) != len)
                                                  {
                                                      Update.printError(Serial);
                                                  }
                                              }
                                              else
                                              {
                                                  Update.printError(Serial);
                                              }
                                              if (final)
                                              {
                                                  if (Update.end(true))
                                                  {
                                                      getRedirectPage(request);
                                                      delay(1000);
                                                      if (webSocket) {
                                                          webSocket->textAll(R"({"type":"otaUpdateFinished"})");
}
                                                      loggerInstance->Info(F("Update done, rebooting..."));
                                                      LittleFS.end();
                                                      if (webSocket) {
                                                          webSocket->closeAll();
}
                                                      server.end();
                                                      ESP.restart();
                                                      return;
                                                  }
                                                  else
                                                  {
                                                      Update.printError(Serial);
                                                  }
                                              } });

Feature *otaUpgrade = new Feature(
    "OTA",
    []()
    {
        server.on("/update", HTTP_GET, getUpdateForm);
        server.on("/update", HTTP_POST, onPostUpdate, onUploadUpdate);
        return FeatureState::RUNNING;
    },
    []() {}, []() { loggerInstance->Info(F("OTA feature stopped")); });

#endif
