#pragma once

#include <Wire.h>

#include "../../config.h"
#include "../Feature.h"
#include "../../CommandInterpreter/CustomCommand.h"
#include "../../CommandInterpreter/CommandParser.h"
#include "../../CommandInterpreter/CommandInterpreter.h"
#include "../../services/WebServer.h"

// helper functions are local to translation unit
static String scanDevices()
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

static String readDevice(uint16_t address, uint16_t size)
{
    // use size_t overload explicitly to avoid ambiguity
    Wire.requestFrom((uint8_t)address, (size_t)size);
    return String(Wire.read());
}

static void writeDevice(uint16_t address, String data)
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

// command and feature objects are defined in cpp
extern CustomCommand *i2cCommand;
extern Feature *i2cFeature;