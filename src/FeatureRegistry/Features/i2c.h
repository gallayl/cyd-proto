#pragma once

#include <Wire.h>
#include <vector>
#include <string>
#include <cstring>
#include <ArduinoJson.h>

#include "../../config.h"
#include "../Feature.h"

inline std::string scanDevices()
{
    JsonDocument doc;
    JsonArray arr = doc.to<JsonArray>();

    byte error;
    byte address;

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

    std::string output;
    serializeJson(doc, output);
    return output;
}

inline std::string readDevice(uint16_t address, uint16_t size)
{
    Wire.requestFrom((uint8_t)address, (size_t)size);

    JsonDocument doc;
    JsonArray arr = doc.to<JsonArray>();
    while (Wire.available() != 0)
    {
        arr.add(Wire.read());
    }

    std::string output;
    serializeJson(doc, output);
    return output;
}

inline void writeDevice(uint16_t address, const std::string &data)
{
    Wire.beginTransmission(address);

    size_t strLen = data.length() + 1;
    std::vector<char> buf(strLen);
    strncpy(buf.data(), data.c_str(), strLen);
    char *p = buf.data();
    char *str;
    while ((str = strtok_r(p, ";", &p)) != NULL)
    {
        if (strncmp(str, "0x", 2) == 0)
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
