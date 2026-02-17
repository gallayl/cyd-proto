#pragma once
#include "cJSON.h"
#include <cstdint>
#include <functional>
#include <mutex>
#include <string>
#include "esp_log.h"
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
    cJSON *getEntries() const
    {
        return _entries;
    }

    void withEntries(std::function<void(cJSON *)> fn) const
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
        _entries = cJSON_CreateArray();
        this->_listenersCount = 0;
        this->_entryCount = 0;
    }

    ~Logger()
    {
        if (_entries)
            cJSON_Delete(_entries);
    }

private:
    mutable std::mutex _entriesMutex;
    std::mutex _listenersMutex;
    cJSON *_entries;
    uint16_t _entryCount;

    uint8_t _listenersCount;
    LogListener _listeners[LOG_LISTENERS_COUNT];

    void handle(const std::string &severity, const std::string &message)
    {
        unsigned long epochTime = getEpochTime();
        std::string utcTime(getUtcTime().c_str());

        this->addEntry(severity, message, epochTime, utcTime);
        if (severity == "E")
            ESP_LOGE("Logger", "[%s] %s - %s", severity.c_str(), utcTime.c_str(), message.c_str());
        else if (severity == "D")
            ESP_LOGD("Logger", "[%s] %s - %s", severity.c_str(), utcTime.c_str(), message.c_str());
        else
            ESP_LOGI("Logger", "[%s] %s - %s", severity.c_str(), utcTime.c_str(), message.c_str());

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

    void addEntry(const std::string &severity, const std::string &message, unsigned long epochTime,
                  const std::string &utcTime)
    {
        std::lock_guard<std::mutex> lock(_entriesMutex);
        if (this->_entryCount >= MAX_LOG_ENTRIES)
        {
            cJSON *first = cJSON_DetachItemFromArray(_entries, 0);
            if (first)
                cJSON_Delete(first);
        }
        else
        {
            this->_entryCount++;
        }

        cJSON *entry = cJSON_CreateObject();
        cJSON_AddStringToObject(entry, "severity", severity.c_str());
        cJSON_AddStringToObject(entry, "message", message.c_str());
        cJSON_AddNumberToObject(entry, "epochTime", epochTime);
        cJSON_AddStringToObject(entry, "isoDateTime", utcTime.c_str());
        cJSON_AddItemToArray(_entries, entry);
    }
};

extern Feature *loggingFeature;
