#pragma once
#include <Arduino.h>

#define LIGHT_SENSOR_PIN 34

inline void initLightSensor()
{
    // No specific initialization needed for the light sensor
}

uint16_t readLightSensor()
{
    return analogRead(LIGHT_SENSOR_PIN);
}