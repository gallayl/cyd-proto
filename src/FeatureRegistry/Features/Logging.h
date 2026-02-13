#pragma once
#include <AsyncJson.h>
#include <ESPAsyncWebServer.h>
#include "../Feature.h"
#include "./Time.h"
#include "../../CommandInterpreter/CustomCommand.h"
#include "../../CommandInterpreter/CommandInterpreter.h"

// forward declaration of global logger so circular includes don't break
class Logger;
extern Logger *LoggerInstance;

// forward declare server type and variable; actual definition lives in WebServer.h
class AsyncWebServer;
extern AsyncWebServer server;

typedef void (*LogListener)(String, String);

#define LOG_LISTENERS_COUNT 10

class Logger
{
public:
    JsonDocument getEntries()
    {
        return this->entries;
    }

    void Info(String message)
    {
        this->handle("I", message);
    }

    void Error(String message)
    {
        this->handle("E", message);
    }

    void Debug(String message)
    {
        this->handle("D", message);
    }

    void AddListener(LogListener listener)
    {
        this->listeners[this->listenersCount] = listener;
        this->listenersCount++;
    }

    Logger()
    {
        this->entries = JsonDocument().to<JsonArray>();
        this->listenersCount = 0;
    }

private:
    JsonDocument entries;

    byte listenersCount;
    LogListener listeners[LOG_LISTENERS_COUNT];

    void handle(String severity, String message)
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

    void addEntry(String severity, String message, unsigned long epochTime, String utcTime)
    {
        JsonObject entry = this->entries.add<JsonObject>();
        entry["severity"] = severity;
        entry["message"] = message;
        entry["epochTime"] = epochTime;
        entry["isoDateTime"] = utcTime;
    }
};

extern Logger *LoggerInstance;

#define LOG_BUFFER_LENGTH 1024

extern CustomCommand *showLogCustomCommand;
extern ArRequestHandlerFunction showLogRequestHandler;

extern Feature *LoggingFeature;
