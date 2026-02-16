#pragma once

#ifdef USE_ESP_IDF
#include "driver/i2c_master.h"
#else
#include <Wire.h>
#endif

#include <vector>
#include <string>
#include <cstring>
#include <ArduinoJson.h>

#include "../../config.h"
#include "../Feature.h"

#ifdef USE_ESP_IDF
extern i2c_master_bus_handle_t i2cBusHandle;
#endif

inline std::string scanDevices()
{
    JsonDocument doc;
    JsonArray arr = doc.to<JsonArray>();

#ifdef USE_ESP_IDF
    for (uint16_t address = 1; address < 127; address++)
    {
        esp_err_t err = i2c_master_probe(i2cBusHandle, address, 50);
        if (err == ESP_OK)
        {
            JsonObject device = arr.add<JsonObject>();
            device["address"] = address;
        }
    }
#else
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
#endif

    std::string output;
    serializeJson(doc, output);
    return output;
}

inline std::string readDevice(uint16_t address, uint16_t size)
{
    JsonDocument doc;
    JsonArray arr = doc.to<JsonArray>();

#ifdef USE_ESP_IDF
    i2c_device_config_t dev_cfg = {};
    dev_cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    dev_cfg.device_address = address;
    dev_cfg.scl_speed_hz = 100000;

    i2c_master_dev_handle_t dev_handle = nullptr;
    esp_err_t err = i2c_master_bus_add_device(i2cBusHandle, &dev_cfg, &dev_handle);
    if (err == ESP_OK && dev_handle != nullptr)
    {
        std::vector<uint8_t> buf(size);
        err = i2c_master_receive(dev_handle, buf.data(), size, 100);
        if (err == ESP_OK)
        {
            for (uint16_t i = 0; i < size; i++)
            {
                arr.add(buf[i]);
            }
        }
        i2c_master_bus_rm_device(dev_handle);
    }
#else
    Wire.requestFrom((uint8_t)address, (size_t)size);

    while (Wire.available() != 0)
    {
        arr.add(Wire.read());
    }
#endif

    std::string output;
    serializeJson(doc, output);
    return output;
}

inline void writeDevice(uint16_t address, const std::string &data)
{
#ifdef USE_ESP_IDF
    i2c_device_config_t dev_cfg = {};
    dev_cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
    dev_cfg.device_address = address;
    dev_cfg.scl_speed_hz = 100000;

    i2c_master_dev_handle_t dev_handle = nullptr;
    esp_err_t err = i2c_master_bus_add_device(i2cBusHandle, &dev_cfg, &dev_handle);
    if (err != ESP_OK || dev_handle == nullptr)
        return;

    std::vector<uint8_t> bytes;

    size_t strLen = data.length() + 1;
    std::vector<char> buf(strLen);
    strncpy(buf.data(), data.c_str(), strLen);
    char *p = buf.data();
    char *str;
    while ((str = strtok_r(p, ";", &p)) != NULL)
    {
        if (strncmp(str, "0x", 2) == 0)
        {
            bytes.push_back((uint8_t)strtol(str, 0, 16));
        }
        else
        {
            for (size_t i = 0; str[i] != '\0'; i++)
            {
                bytes.push_back((uint8_t)str[i]);
            }
        }
    }

    if (!bytes.empty())
    {
        i2c_master_transmit(dev_handle, bytes.data(), bytes.size(), 100);
    }
    i2c_master_bus_rm_device(dev_handle);
#else
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
#endif
}

extern Feature *i2cFeature;
