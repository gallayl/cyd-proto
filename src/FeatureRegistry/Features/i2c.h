#pragma once

#include <Wire.h>
#include <vector>

#include "../../config.h"
#include "../Feature.h"
#include "../../CommandInterpreter/CustomCommand.h"
#include "../../CommandInterpreter/CommandParser.h"
#include "../../CommandInterpreter/CommandInterpreter.h"
#include "../../services/WebServer.h"

// helper functions are local to translation unit
static String scanDevices()
{
    JsonDocument doc;
    JsonArray arr = doc.to<JsonArray>();

    byte error, address;

    for (address = 1; address < 127; address++)
    {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();

        if (error == 0)
        {
            JsonObject device = arr.add<JsonObject>();
            device["address"] = address;
        }
    }

    String output;
    serializeJson(doc, output);
    return output;
}

static String readDevice(uint16_t address, uint16_t size)
{
    // use size_t overload explicitly to avoid ambiguity
    Wire.requestFrom((uint8_t)address, (size_t)size);
    return String(Wire.read());
}

static void writeDevice(uint16_t address, const String &data)
{
    Wire.beginTransmission(address);

    size_t str_len = data.length() + 1;
    std::vector<char> buf(str_len);
    data.toCharArray(buf.data(), str_len);
    char *p = buf.data();
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