#include "../../config.h"

#if ENABLE_OTA

#include "OTA.h"
#include "../../mime.h"
#include "./Logging.h"
#include "../../utils/System.h"
#include "../../fs/LittleFsInit.h"

#ifdef USE_ESP_IDF

#include <esp_log.h>
#include <esp_ota_ops.h>
#include <esp_partition.h>
#include "../../services/WebServer.h"
#include "../../services/WebSocketServer.h"
#include "../../utils/MultipartParser.h"

static const char *OTA_TAG = "OTA";

// --- GET /update - serve HTML form ---

static esp_err_t otaGetFormHandler(httpd_req_t *req)
{
    const char *html = "<form method='POST' action='/update' accept='application/octet-stream' "
                       "enctype='multipart/form-data'>"
                       "<input type='file' name='update'>"
                       "<input type='submit' value='Update'>"
                       "</form>";
    httpd_resp_set_type(req, MIME_HTML);
    return httpd_resp_send(req, html, HTTPD_RESP_USE_STRLEN);
}

// --- POST /update - receive firmware and flash via esp_ota_ops ---

static esp_err_t otaPostHandler(httpd_req_t *req)
{
    esp_ota_handle_t otaHandle = 0;
    const esp_partition_t *otaPartition = nullptr;
    bool otaStarted = false;
    std::string otaError;

    esp_err_t ret = parseMultipartRequest(
        req,
        nullptr, // no text fields expected
        [&](const std::string & /*fieldName*/, const std::string & /*fileName*/, const uint8_t *data, size_t len,
            bool isFirst, bool isFinal) -> bool
        {
            if (isFirst)
            {
                otaPartition = esp_ota_get_next_update_partition(nullptr);
                if (!otaPartition)
                {
                    otaError = "No OTA update partition found";
                    ESP_LOGE(OTA_TAG, "%s", otaError.c_str());
                    return false;
                }

                esp_err_t err = esp_ota_begin(otaPartition, OTA_WITH_SEQUENTIAL_WRITES, &otaHandle);
                if (err != ESP_OK)
                {
                    otaError = std::string("esp_ota_begin failed: ") + esp_err_to_name(err);
                    ESP_LOGE(OTA_TAG, "%s", otaError.c_str());
                    return false;
                }

                otaStarted = true;
                loggerInstance->Info("Starting OTA update...");
                wsBroadcast(R"({"type":"otaUpdateStarted"})");
            }

            if (data && len > 0)
            {
                esp_err_t err = esp_ota_write(otaHandle, data, len);
                if (err != ESP_OK)
                {
                    otaError = std::string("esp_ota_write failed: ") + esp_err_to_name(err);
                    ESP_LOGE(OTA_TAG, "%s", otaError.c_str());
                    return false;
                }
            }

            if (isFinal)
            {
                esp_err_t err = esp_ota_end(otaHandle);
                if (err != ESP_OK)
                {
                    otaError = std::string("esp_ota_end failed: ") + esp_err_to_name(err);
                    ESP_LOGE(OTA_TAG, "%s", otaError.c_str());
                    otaStarted = false;
                    return false;
                }

                err = esp_ota_set_boot_partition(otaPartition);
                if (err != ESP_OK)
                {
                    otaError = std::string("esp_ota_set_boot_partition failed: ") + esp_err_to_name(err);
                    ESP_LOGE(OTA_TAG, "%s", otaError.c_str());
                    return false;
                }
            }
            return true;
        });

    if (!otaError.empty() || ret != ESP_OK)
    {
        if (otaStarted)
            esp_ota_abort(otaHandle);

        std::string errResp = otaError.empty() ? "OTA update failed" : otaError;
        httpd_resp_set_type(req, MIME_PLAIN_TEXT);
        httpd_resp_set_status(req, "500 Internal Server Error");
        return httpd_resp_send(req, errResp.c_str(), HTTPD_RESP_USE_STRLEN);
    }

    // Send redirect page then reboot
    const char *html = "<html><head><meta http-equiv=\"refresh\" content=\"30\"></head>"
                       "<body>Update done, page will be refreshed.</body></html>";
    httpd_resp_set_type(req, MIME_HTML);
    httpd_resp_set_hdr(req, "Refresh", REFRESH_TIMEOUT_AFTER_UPDATE);
    httpd_resp_send(req, html, HTTPD_RESP_USE_STRLEN);

    wsBroadcast(R"({"type":"otaUpdateFinished"})");
    loggerInstance->Info("Update done, rebooting...");

    vTaskDelay(pdMS_TO_TICKS(1000));
    deinitLittleFs();
    stopWebServer();
    systemRestart();

    return ESP_OK;
}

Feature *otaUpgrade = new Feature(
    "OTA",
    []()
    {
        httpd_handle_t server = getHttpServer();
        if (!server)
        {
            loggerInstance->Error("OTA: HTTP server not available");
            return FeatureState::ERROR;
        }

        const httpd_uri_t getUri = {
            .uri = "/update", .method = HTTP_GET, .handler = otaGetFormHandler, .user_ctx = nullptr};
        httpd_register_uri_handler(server, &getUri);

        const httpd_uri_t postUri = {
            .uri = "/update", .method = HTTP_POST, .handler = otaPostHandler, .user_ctx = nullptr};
        httpd_register_uri_handler(server, &postUri);

        loggerInstance->Info("OTA endpoints registered");
        return FeatureState::RUNNING;
    },
    []() {}, []() { loggerInstance->Info("OTA feature stopped"); });

#else // Arduino

#include "../../services/WebSocketServer.h"
#include "../../services/WebServer.h"

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
                                  Update.printError(Serial);
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
                                                      vTaskDelay(pdMS_TO_TICKS(1000));
                                                      if (webSocket) {
                                                          webSocket->textAll(R"({"type":"otaUpdateFinished"})");
}
                                                      loggerInstance->Info("Update done, rebooting...");
                                                      deinitLittleFs();
                                                      if (webSocket) {
                                                          webSocket->closeAll();
}
                                                      server.end();
                                                      systemRestart();
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
    []() {}, []() { loggerInstance->Info("OTA feature stopped"); });

#endif // USE_ESP_IDF

#endif // ENABLE_OTA
