#pragma once
#include <ArduinoJson.h>
#include <functional>
#include <mutex>
#include "../../config.h"
#include "../Feature.h"
#include "./Time.h"
#include "../../ActionRegistry/FeatureAction.h"

// forward declaration of global logger so circular includes don't break
class Logger;
extern Logger *LoggerInstance;

typedef void (*LogListener)(const String &, const String &);

#define LOG_LISTENERS_COUNT 10
#define MAX_LOG_ENTRIES 100

class Logger
{
public:
    const JsonDocument &getEntries() const
    {
        return this->entries;
    }

    void withEntries(std::function<void(const JsonDocument &)> fn) const
    {
        std::lock_guard<std::mutex> lock(entriesMutex);
        fn(entries);
    }

    void Info(const String &message)
    {
        this->handle("I", message);
    }

    void Error(const String &message)
    {
        this->handle("E", message);
    }

    void Debug(const String &message)
    {
        this->handle("D", message);
    }

    void AddListener(LogListener listener)
    {
        if (this->listenersCount < LOG_LISTENERS_COUNT)
        {
            this->listeners[this->listenersCount] = listener;
            this->listenersCount++;
        }
    }

    Logger()
    {
        this->entries.to<JsonArray>();
        this->listenersCount = 0;
        this->entryCount = 0;
    }

private:
    mutable std::mutex entriesMutex;
    JsonDocument entries;
    uint16_t entryCount;

    byte listenersCount;
    LogListener listeners[LOG_LISTENERS_COUNT];

    void handle(const String &severity, const String &message)
    {
        unsigned long epochTime = getEpochTime();
        String utcTime = getUtcTime();

        this->addEntry(severity, message, epochTime, utcTime);
        Serial.print(F("["));
        Serial.print(severity);
        Serial.print(F("] "));
        Serial.print(utcTime);
        Serial.print(F(" - "));
        Serial.println(message);
        for (byte i = 0; i < this->listenersCount; i++)
        {
            this->listeners[i](severity, message);
        }
    }

    uint16_t compactCounter{0};

    void compactEntries()
    {
        JsonDocument fresh;
        JsonArray freshArr = fresh.to<JsonArray>();
        JsonArrayConst oldArr = this->entries.as<JsonArrayConst>();
        for (JsonObjectConst obj : oldArr)
        {
            freshArr.add(obj);
        }
        this->entries = std::move(fresh);
    }

    void addEntry(const String &severity, const String &message, unsigned long epochTime, const String &utcTime)
    {
        std::lock_guard<std::mutex> lock(entriesMutex);
        if (this->entryCount >= MAX_LOG_ENTRIES)
        {
            JsonArray arr = this->entries.as<JsonArray>();
            arr.remove(0);

            // periodically compact to reclaim memory from removed entries
            if (++compactCounter >= 20)
            {
                compactCounter = 0;
                compactEntries();
            }
        }
        else
        {
            this->entryCount++;
        }

        JsonObject entry = this->entries.as<JsonArray>().add<JsonObject>();
        entry["severity"] = severity;
        entry["message"] = message;
        entry["epochTime"] = epochTime;
        entry["isoDateTime"] = utcTime;
    }
};

extern Logger *LoggerInstance;

extern Feature *LoggingFeature;
