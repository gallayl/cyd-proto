#pragma once

#include <Wire.h>
#include <vector>
#include <ArduinoJson.h>

#include "../../config.h"
#include "../Feature.h"
#include "../../ActionRegistry/FeatureAction.h"
#include "../../CommandInterpreter/CommandParser.h"

inline String scanDevices()
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

inline String readDevice(uint16_t address, uint16_t size)
{
    Wire.requestFrom((uint8_t)address, (size_t)size);

    JsonDocument doc;
    JsonArray arr = doc.to<JsonArray>();
    while (Wire.available())
    {
        arr.add(Wire.read());
    }

    String output;
    serializeJson(doc, output);
    return output;
}

inline void writeDevice(uint16_t address, const String &data)
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

extern Feature *i2cFeature;
