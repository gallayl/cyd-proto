#pragma once

#ifdef USE_ESP_IDF
#include "driver/ledc.h"
#include <cstdint>
#else
#include <Arduino.h>
#endif

#define RGB_LED_R_PIN 4
#define RGB_LED_G_PIN 16
#define RGB_LED_B_PIN 17

#ifdef USE_ESP_IDF

inline void initRgbLed()
{
    ledc_timer_config_t timer_conf = {};
    timer_conf.speed_mode = LEDC_LOW_SPEED_MODE;
    timer_conf.duty_resolution = LEDC_TIMER_8_BIT;
    timer_conf.timer_num = LEDC_TIMER_0;
    timer_conf.freq_hz = 5000;
    timer_conf.clk_cfg = LEDC_AUTO_CLK;
    ledc_timer_config(&timer_conf);

    const int pins[] = {RGB_LED_R_PIN, RGB_LED_G_PIN, RGB_LED_B_PIN};
    const ledc_channel_t channels[] = {LEDC_CHANNEL_0, LEDC_CHANNEL_1, LEDC_CHANNEL_2};

    for (int i = 0; i < 3; i++)
    {
        ledc_channel_config_t ch = {};
        ch.gpio_num = pins[i];
        ch.speed_mode = LEDC_LOW_SPEED_MODE;
        ch.channel = channels[i];
        ch.intr_type = LEDC_INTR_DISABLE;
        ch.timer_sel = LEDC_TIMER_0;
        ch.duty = 255; // active-low: 255 duty = LED off
        ch.hpoint = 0;
        ledc_channel_config(&ch);
    }
}

inline void setRgbLedColor(uint8_t r, uint8_t g, uint8_t b)
{
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 255 - r);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, 255 - g);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2, 255 - b);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2);
}

#else

inline void setRgbLedColor(uint8_t r, uint8_t g, uint8_t b)
{
    analogWrite(RGB_LED_R_PIN, 255 - r); // Invert the value since the LED is active low
    analogWrite(RGB_LED_G_PIN, 255 - g);
    analogWrite(RGB_LED_B_PIN, 255 - b);
}

inline void initRgbLed()
{
    pinMode(RGB_LED_R_PIN, OUTPUT);
    pinMode(RGB_LED_G_PIN, OUTPUT);
    pinMode(RGB_LED_B_PIN, OUTPUT);

    // turn off the LED by default (active-low: HIGH = off)
    digitalWrite(RGB_LED_R_PIN, HIGH);
    digitalWrite(RGB_LED_G_PIN, HIGH);
    digitalWrite(RGB_LED_B_PIN, HIGH);
}

#endif
