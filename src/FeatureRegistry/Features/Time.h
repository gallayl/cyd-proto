#pragma once
#include <WiFiUdp.h>

#include <ctime>
#include "../Feature.h"

#define MY_NTP_SERVER "pool.ntp.org"
// ESP32 configTime takes offsets in seconds, so we keep our TZ string separate
#define MY_TZ "CET-1CEST,M3.5.0,M10.5.0/3"

// time helpers (implemented in Time.cpp)
time_t getEpochTime();
String getUtcTime();

// Feature object is instantiated in Time.cpp
extern Feature *timeFeature;
