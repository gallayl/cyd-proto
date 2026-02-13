#include "OTA.h"
#include "../../services/WebSocketServer.h"  // ensure webSocket is declared (fixed relative path)

// redundant extern in this translation unit to satisfy IntelliSense
extern AsyncWebSocket *webSocket;

ArRequestHandlerFunction getUpdateForm = ([](AsyncWebServerRequest *request)
                                          { request->send(200, MIME_html, F("<form method='POST' action='/update' accept='application/octet-stream' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>")); });

ArRequestHandlerFunction getRedirectPage = ([](AsyncWebServerRequest *request)
                                            {
                                                AsyncWebServerResponse *response = request->beginResponse(200, MIME_html, "<html><head><meta http-equiv=\"refresh\" content=\"30\"></head><body>Update done, page will be refreshed.</body></html>");
                                                response->addHeader("Refresh", REFRESH_TIMEOUT_AFTER_UPDATE);
                                                request->send(response);
                                            });

ArRequestHandlerFunction onPostUpdate = ([](AsyncWebServerRequest *request)
                                         {
                                             boolean shouldReboot = !Update.hasError();
                                             AsyncWebServerResponse *response = request->beginResponse(200, MIME_plainText, shouldReboot ? "OK" : "FAIL");
                                             response->addHeader("Connection", "close");
                                             request->send(response);
                                         });

ArUploadHandlerFunction onUploadUpdate = ([](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
                                          {
                                              // ESP32 Update library does not support runAsync
                                              if (Update.hasError())
                                              {
                                                  Update.printError(Serial);
                                                  request->send(500, MIME_plainText, "Update error");
                                                  return;
                                              }
                                              if (!index)
                                              {
                                                  LoggerInstance->Info("Starting OTA update...");
                                                  webSocket->textAll("{\"type\":\"odaUpdateStarted\"}");
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
                                                      webSocket->textAll("{\"type\":\"odaUpdateFinished\"}");
                                                      LoggerInstance->Info("Update done, rebooting...");
                                                      LittleFS.end();
                                                      webSocket->closeAll();
                                                      server.end();
                                                      ESP.restart();
                                                      return;
                                                  }
                                                  else
                                                  {
                                                      Update.printError(Serial);
                                                  }
                                              }
                                          });

Feature *OtaUpgrade = new Feature("OTA", []()
                                  {
                                      server.on("/update", HTTP_GET, getUpdateForm);
                                      server.on("/update", HTTP_POST, onPostUpdate, onUploadUpdate);
                                      return FeatureState::RUNNING;
                                  }, []() {});
