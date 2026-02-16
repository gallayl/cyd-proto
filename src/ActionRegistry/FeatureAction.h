#pragma once

#include <string>
#include <cstdint>

enum class Transport : uint8_t
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

using ActionHandler = std::string (*)(const std::string &command);

struct FeatureAction
{
    std::string name;
    std::string type = "GET";
    ActionHandler handler;
    TransportConfig transports;
};
