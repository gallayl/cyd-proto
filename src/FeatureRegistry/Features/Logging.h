#pragma once
#ifdef USE_ESP_IDF
#include "cJSON.h"
#else
#include <ArduinoJson.h>
#endif
#include <cstdint>
#include <functional>
#include <mutex>
#include <string>
#ifdef USE_ESP_IDF
#include "esp_log.h"
#endif
#include "../../config.h"
#include "../Feature.h"
#include "./Time.h"

// forward declaration of global logger so circular includes don't break
class Logger;
extern Logger *loggerInstance;

using LogListener = void (*)(const std::string &, const std::string &);

#define LOG_LISTENERS_COUNT 10
#define MAX_LOG_ENTRIES 100

class Logger
{
public:
#ifdef USE_ESP_IDF
    cJSON *getEntries() const
    {
        return _entries;
    }

    void withEntries(std::function<void(cJSON *)> fn) const
    {
        std::lock_guard<std::mutex> lock(_entriesMutex);
        fn(_entries);
    }
#else
    const JsonDocument &getEntries() const
    {
        return this->_entries;
    }

    void withEntries(std::function<void(const JsonDocument &)> fn) const
    {
        std::lock_guard<std::mutex> lock(_entriesMutex);
        fn(_entries);
    }
#endif

    void Info(const std::string &message)
    {
        this->handle("I", message);
    }

    void Error(const std::string &message)
    {
        this->handle("E", message);
    }

    void Debug(const std::string &message)
    {
        this->handle("D", message);
    }

    void AddListener(LogListener listener)
    {
        std::lock_guard<std::mutex> lock(_listenersMutex);
        if (this->_listenersCount < LOG_LISTENERS_COUNT)
        {
            this->_listeners[this->_listenersCount] = listener;
            this->_listenersCount++;
        }
    }

    Logger()
    {
#ifdef USE_ESP_IDF
        _entries = cJSON_CreateArray();
#else
        this->_entries.to<JsonArray>();
#endif
        this->_listenersCount = 0;
        this->_entryCount = 0;
    }

    ~Logger()
    {
#ifdef USE_ESP_IDF
        if (_entries)
            cJSON_Delete(_entries);
#endif
    }

private:
    mutable std::mutex _entriesMutex;
    std::mutex _listenersMutex;
#ifdef USE_ESP_IDF
    cJSON *_entries;
#else
    JsonDocument _entries;
#endif
    uint16_t _entryCount;

    uint8_t _listenersCount;
    LogListener _listeners[LOG_LISTENERS_COUNT];

    void handle(const std::string &severity, const std::string &message)
    {
        unsigned long epochTime = getEpochTime();
        std::string utcTime(getUtcTime().c_str());

        this->addEntry(severity, message, epochTime, utcTime);
#ifdef USE_ESP_IDF
        if (severity == "E")
            ESP_LOGE("Logger", "[%s] %s - %s", severity.c_str(), utcTime.c_str(), message.c_str());
        else if (severity == "D")
            ESP_LOGD("Logger", "[%s] %s - %s", severity.c_str(), utcTime.c_str(), message.c_str());
        else
            ESP_LOGI("Logger", "[%s] %s - %s", severity.c_str(), utcTime.c_str(), message.c_str());
#else
        Serial.print("[");
        Serial.print(severity.c_str());
        Serial.print("] ");
        Serial.print(utcTime.c_str());
        Serial.print(" - ");
        Serial.println(message.c_str());
#endif

        LogListener listenersCopy[LOG_LISTENERS_COUNT];
        uint8_t count;
        {
            std::lock_guard<std::mutex> lock(_listenersMutex);
            count = this->_listenersCount;
            memcpy(listenersCopy, this->_listeners, sizeof(LogListener) * count);
        }
        for (uint8_t i = 0; i < count; i++)
        {
            listenersCopy[i](severity, message);
        }
    }

#ifndef USE_ESP_IDF
    uint16_t _compactCounter{0};

    void compactEntries()
    {
        JsonDocument fresh;
        JsonArray freshArr = fresh.to<JsonArray>();
        JsonArrayConst oldArr = this->_entries.as<JsonArrayConst>();
        for (JsonObjectConst obj : oldArr)
        {
            freshArr.add(obj);
        }
        this->_entries = std::move(fresh);
    }
#endif

    void addEntry(const std::string &severity, const std::string &message, unsigned long epochTime,
                  const std::string &utcTime)
    {
        std::lock_guard<std::mutex> lock(_entriesMutex);
        if (this->_entryCount >= MAX_LOG_ENTRIES)
        {
#ifdef USE_ESP_IDF
            cJSON *first = cJSON_DetachItemFromArray(_entries, 0);
            if (first)
                cJSON_Delete(first);
#else
            JsonArray arr = this->_entries.as<JsonArray>();
            arr.remove(0);

            // periodically compact to reclaim memory from removed entries
            if (++_compactCounter >= 20)
            {
                _compactCounter = 0;
                compactEntries();
            }
#endif
        }
        else
        {
            this->_entryCount++;
        }

#ifdef USE_ESP_IDF
        cJSON *entry = cJSON_CreateObject();
        cJSON_AddStringToObject(entry, "severity", severity.c_str());
        cJSON_AddStringToObject(entry, "message", message.c_str());
        cJSON_AddNumberToObject(entry, "epochTime", epochTime);
        cJSON_AddStringToObject(entry, "isoDateTime", utcTime.c_str());
        cJSON_AddItemToArray(_entries, entry);
#else
        JsonObject entry = this->_entries.as<JsonArray>().add<JsonObject>();
        entry["severity"] = severity;
        entry["message"] = message;
        entry["epochTime"] = epochTime;
        entry["isoDateTime"] = utcTime;
#endif
    }
};

extern Feature *loggingFeature;
