#pragma once

#include <Arduino.h>

#define RGB_LED_R_PIN 4
#define RGB_LED_G_PIN 16
#define RGB_LED_B_PIN 17

void setRgbLedColor(uint8_t r, uint8_t g, uint8_t b)
{
    analogWrite(RGB_LED_R_PIN, 255 - r); // Invert the value since the LED is active low
    analogWrite(RGB_LED_G_PIN, 255 - g);
    analogWrite(RGB_LED_B_PIN, 255 - b);
}

void initRgbLed()
{
    pinMode(RGB_LED_R_PIN, OUTPUT);
    pinMode(RGB_LED_G_PIN, OUTPUT);
    pinMode(RGB_LED_B_PIN, OUTPUT);

    // turn off the LED by default
    digitalWrite(RGB_LED_R_PIN, LOW);
    digitalWrite(RGB_LED_G_PIN, LOW);
    digitalWrite(RGB_LED_B_PIN, LOW);
}