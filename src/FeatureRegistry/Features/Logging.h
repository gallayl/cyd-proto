#pragma once
#include <AsyncJson.h>
#include "../../config.h"
#include "../Feature.h"
#include "./Time.h"
#include "../../CommandInterpreter/CustomCommand.h"
#include "../../CommandInterpreter/CommandInterpreter.h"

#if ENABLE_WEBSERVER
#include <ESPAsyncWebServer.h>
// forward declare server type and variable; actual definition lives in WebServer.h
class AsyncWebServer;
extern AsyncWebServer server;
#endif

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

    void addEntry(const String &severity, const String &message, unsigned long epochTime, const String &utcTime)
    {
        if (this->entryCount >= MAX_LOG_ENTRIES)
        {
            JsonArray arr = this->entries.as<JsonArray>();
            arr.remove(0);
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

extern CustomCommand *showLogCustomCommand;

#if ENABLE_WEBSERVER
extern ArRequestHandlerFunction showLogRequestHandler;
#endif

extern Feature *LoggingFeature;
