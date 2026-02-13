#pragma once

#include <Wire.h>

#include "../../config.h"
#include "../Feature.h"
#include "../../CommandInterpreter/CustomCommand.h"
#include "../../CommandInterpreter/CommandParser.h"
#include "../../CommandInterpreter/CommandInterpreter.h"
#include "../../services/WebServer.h"

String scanDevices()
{
    JsonDocument doc = JsonDocument().as<JsonArray>();

    Wire.begin();
    byte error, address;

    for (address = 1; address < 127; address++)
    {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();

        if (error == 0)
        {
            JsonObject device = doc.as<JsonArray>().add<JsonObject>();
            device["address"] = address;
        }
    }

    char buffer[JSON_BUFFER_SIZE];
    serializeJson(doc, buffer);
    return String(buffer);
}

String readDevice(uint16_t address, uint16_t size)
{
    Wire.requestFrom(address, size);
    return String(Wire.read());
}

void writeDevice(uint16_t address, String data)
{
    Wire.beginTransmission(address);

    int str_len = data.length() + 1;
    char buf[str_len];
    data.toCharArray(buf, str_len);
    char *p = buf;
    char *str;
    while ((str = strtok_r(p, ";", &p)) != NULL)
    {
        if (String(str).startsWith("0x"))
        {
            Wire.write(strtol(str, 0, 16));
        }
        else
        {
            Wire.write(str);
        }
    }

    Wire.endTransmission();
}

CustomCommand *i2cCommand = new CustomCommand("i2c", [](String command)
                                              {
    String sub = CommandParser::GetCommandParameter(command, 1);
    if (sub == "scan")
    {
        return scanDevices();
    }
    else if (sub == "read")
    {
        uint16_t address = CommandParser::GetCommandParameter(command, 2).toInt();
        uint16_t size = CommandParser::GetCommandParameter(command, 3).toInt();
        return readDevice(address, size);
    }
    else if (sub == "write")
    {
        uint16_t address = strtol(CommandParser::GetCommandParameter(command, 2).c_str(), 0, 16);
        command.replace("i2c write ", "");
        writeDevice(address, command);
        return String("Writed.");
    }

    String fallback = "The awailable I2C Commands are: scan, read, write";
    return fallback; });

Feature *i2cFeature = new Feature("i2c", []()
                                  {


    Wire.begin();

    CommandInterpreterInstance->RegisterCommand(*i2cCommand);

    server.on("/i2c", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, MIME_json, scanDevices());
    });

    return FeatureState::RUNNING; }, []() {});