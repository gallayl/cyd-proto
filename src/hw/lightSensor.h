#pragma once

#include "esp_adc/adc_oneshot.h"
#include <cstdint>

#define LIGHT_SENSOR_PIN 34

// GPIO 34 = ADC1_CHANNEL_6 on ESP32
#define LIGHT_SENSOR_ADC_CHANNEL ADC_CHANNEL_6

static adc_oneshot_unit_handle_t s_light_adc_handle = nullptr;

inline void initLightSensor()
{
    adc_oneshot_unit_init_cfg_t init_cfg = {};
    init_cfg.unit_id = ADC_UNIT_1;
    adc_oneshot_new_unit(&init_cfg, &s_light_adc_handle);

    adc_oneshot_chan_cfg_t chan_cfg = {};
    chan_cfg.atten = ADC_ATTEN_DB_12;
    chan_cfg.bitwidth = ADC_BITWIDTH_DEFAULT;
    adc_oneshot_config_channel(s_light_adc_handle, LIGHT_SENSOR_ADC_CHANNEL, &chan_cfg);
}

inline uint16_t readLightSensor()
{
    int raw = 0;
    if (s_light_adc_handle != nullptr)
    {
        adc_oneshot_read(s_light_adc_handle, LIGHT_SENSOR_ADC_CHANNEL, &raw);
    }
    return (uint16_t)raw;
}
