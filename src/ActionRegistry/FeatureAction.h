#pragma once

#include <Arduino.h>

enum class Transport
{
    CLI,
    REST,
    WS,
    SCRIPTING
};

struct TransportConfig
{
    bool cli = true;
    bool rest = false;
    bool ws = true;
    bool scripting = true;
};

typedef String (*ActionHandler)(const String &command);

struct FeatureAction
{
    String name;
    String type = "GET";
    ActionHandler handler;
    TransportConfig transports;
};
