#include "../../config.h"

#if ENABLE_OTA

#include "OTA.h"
#include "../../services/WebSocketServer.h" // ensure webSocket is declared (fixed relative path)
#include "../../mime.h"
#include "./Logging.h"
#include "../../services/WebServer.h"
#include "../../utils/System.h"
#ifdef USE_ESP_IDF
#include "esp_log.h"
#endif

// redundant extern in this translation unit to satisfy IntelliSense

ArRequestHandlerFunction getUpdateForm = ([](AsyncWebServerRequest *request)
                                          { request->send(200, MIME_HTML, "<form method='POST' action='/update' accept='application/octet-stream' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>"); });

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
#ifdef USE_ESP_IDF
                                  ESP_LOGE("OTA", "Update error");
#else
                                  Update.printError(Serial);
#endif
                                  request->send(500, MIME_PLAIN_TEXT, "Update error");
                                  return;
                              }
                                              if (!index)
                                              {
                                                  loggerInstance->Info("Starting OTA update...");
                                                  if (webSocket) {
                                                      webSocket->textAll(R"({"type":"otaUpdateStarted"})");
}
                                  if (!Update.begin(request->contentLength(), U_FLASH))
                                  {
#ifdef USE_ESP_IDF
                                      ESP_LOGE("OTA", "Update begin failed");
#else
                                      Update.printError(Serial);
#endif
                                  }
                                              }
                              if (!Update.hasError())
                              {
                                  if (Update.write(data, len) != len)
                                  {
#ifdef USE_ESP_IDF
                                      ESP_LOGE("OTA", "Update write failed");
#else
                                      Update.printError(Serial);
#endif
                                  }
                              }
                              else
                              {
#ifdef USE_ESP_IDF
                                  ESP_LOGE("OTA", "Update error during write");
#else
                                  Update.printError(Serial);
#endif
                              }
                                              if (final)
                                              {
                                                  if (Update.end(true))
                                                  {
                                                      getRedirectPage(request);
                                                      vTaskDelay(pdMS_TO_TICKS(1000));
                                                      if (webSocket) {
                                                          webSocket->textAll(R"({"type":"otaUpdateFinished"})");
}
                                                      loggerInstance->Info("Update done, rebooting...");
                                                      LittleFS.end();
                                                      if (webSocket) {
                                                          webSocket->closeAll();
}
                                                      server.end();
                                                      systemRestart();
                                                      return;
                                                  }
                                  else
                                  {
#ifdef USE_ESP_IDF
                                      ESP_LOGE("OTA", "Update end failed");
#else
                                      Update.printError(Serial);
#endif
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
    []() {}, []() { loggerInstance->Info("OTA feature stopped"); });

#endif
