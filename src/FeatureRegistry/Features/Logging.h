#pragma once
#include <ArduinoJson.h>
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
    const JsonDocument &getEntries() const
    {
        return this->_entries;
    }

    void withEntries(std::function<void(const JsonDocument &)> fn) const
    {
        std::lock_guard<std::mutex> lock(_entriesMutex);
        fn(_entries);
    }

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
        this->_entries.to<JsonArray>();
        this->_listenersCount = 0;
        this->_entryCount = 0;
    }

private:
    mutable std::mutex _entriesMutex;
    std::mutex _listenersMutex;
    JsonDocument _entries;
    uint16_t _entryCount;

    byte _listenersCount;
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
        byte count;
        {
            std::lock_guard<std::mutex> lock(_listenersMutex);
            count = this->_listenersCount;
            memcpy(listenersCopy, this->_listeners, sizeof(LogListener) * count);
        }
        for (byte i = 0; i < count; i++)
        {
            listenersCopy[i](severity, message);
        }
    }

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

    void addEntry(const std::string &severity, const std::string &message, unsigned long epochTime,
                  const std::string &utcTime)
    {
        std::lock_guard<std::mutex> lock(_entriesMutex);
        if (this->_entryCount >= MAX_LOG_ENTRIES)
        {
            JsonArray arr = this->_entries.as<JsonArray>();
            arr.remove(0);

            // periodically compact to reclaim memory from removed entries
            if (++_compactCounter >= 20)
            {
                _compactCounter = 0;
                compactEntries();
            }
        }
        else
        {
            this->_entryCount++;
        }

        JsonObject entry = this->_entries.as<JsonArray>().add<JsonObject>();
        entry["severity"] = severity;
        entry["message"] = message;
        entry["epochTime"] = epochTime;
        entry["isoDateTime"] = utcTime;
    }
};

extern Feature *loggingFeature;
