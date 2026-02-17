#pragma once

#include "driver/i2c_master.h"
#include "cJSON.h"
#include "../../utils/CJsonHelper.h"

#include <vector>
#include <string>
#include <cstring>

#include "../../config.h"
#include "../Feature.h"

extern i2c_master_bus_handle_t i2cBusHandle;

inline std::string scanDevices()
{
    cJSON *arr = cJSON_CreateArray();

    for (uint16_t address = 1; address < 127; address++)
    {
        esp_err_t err = i2c_master_probe(i2cBusHandle, address, 50);
        if (err == ESP_OK)
        {
            cJSON *device = cJSON_CreateObject();
            cJSON_AddNumberToObject(device, "address", address);
            cJSON_AddItemToArray(arr, device);
        }
    }

    std::string output = cJsonToString(arr);
    cJSON_Delete(arr);
    return output;
}

inline std::string readDevice(uint16_t address, uint16_t size)
{
    cJSON *arr = cJSON_CreateArray();

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
                cJSON_AddItemToArray(arr, cJSON_CreateNumber(buf[i]));
            }
        }
        i2c_master_bus_rm_device(dev_handle);
    }

    std::string output = cJsonToString(arr);
    cJSON_Delete(arr);
    return output;
}

inline void writeDevice(uint16_t address, const std::string &data)
{
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
}

extern Feature *i2cFeature;
