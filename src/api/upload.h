#pragma once
#include "../FeatureRegistry/Features/Logging.h"
#include <Update.h>

ArRequestHandlerFunction onPostUploadFiles = ([](AsyncWebServerRequest *request)
                                              {
    boolean shouldReboot = !Update.hasError();
    AsyncWebServerResponse *response = request->beginResponse(200, MIME_plainText, shouldReboot ? "OK" : "FAIL");
    response->addHeader("Connection", "close");
    request->send(response); });

ArUploadHandlerFunction uploadFiles = ([](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
                                       {
        fs::File file = LittleFS.open("/"+filename, "a");

        if (index == 0 && file.position() > 0)
        {
            file.seek(0, fs::SeekSet);
        }

        if (!file)
        {
            LoggerInstance->Error("Failed to open file for writing");
            request->send(500, MIME_json, "{\"error\":\"Failed to open file for writing\"}");
            return;
        }
        file.write(data, len);
        file.close();

    if (final)
    {
        LoggerInstance->Info("Upload finished");
        request->send(200, MIME_json, "{\"status\":\"ok\"}");
    }; });