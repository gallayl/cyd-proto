#pragma once

#include <cstdint>
#include "esp_log.h"
#include <mutex>
#include <string>
#include "../config.h"
#include "FeatureAction.h"
#include "../CommandInterpreter/CommandParser.h"
#include "../utils/StringUtil.h"

#define ACTIONS_SIZE 24

class ActionRegistry
{
private:
    uint8_t _registeredActionsCount = 0;
    FeatureAction *_actions[ACTIONS_SIZE] = {};
    std::mutex _mutex;

    bool isTransportEnabled(const FeatureAction *action, Transport transport) const
    {
        switch (transport)
        {
        case Transport::CLI:
            return action->transports.cli;
        case Transport::REST:
            return action->transports.rest;
        case Transport::WS:
            return action->transports.ws;
        case Transport::SCRIPTING:
            return action->transports.scripting;
        }
        return false;
    }

public:
    void registerAction(FeatureAction *action)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_registeredActionsCount >= ACTIONS_SIZE)
        {
            ESP_LOGE("ActionRegistry", "Action registry full, cannot register");
            return;
        }
        _actions[_registeredActionsCount] = action;
        _registeredActionsCount++;
    }

    std::string execute(const std::string &command, Transport transport)
    {
        ActionHandler handler = nullptr;
        {
            std::lock_guard<std::mutex> lock(_mutex);
            for (uint8_t i = 0; i < _registeredActionsCount; i++)
            {
                const std::string &name = _actions[i]->name;
                if (command == name || StringUtil::startsWith(command, name + " "))
                {
                    if (!isTransportEnabled(_actions[i], transport))
                    {
                        return "{\"error\": \"Action '" + name + "' not available on this transport\"}";
                    }
                    handler = _actions[i]->handler;
                    break;
                }
            }
        }
        if (handler != nullptr)
        {
            return handler(command);
        }
        return "{\"message\": \"Unknown action: " + CommandParser::getCommandName(command) +
               ".\", \"availableActions\": \"" + getAvailableActions(transport) + "\"}";
    }

    std::string getAvailableActions(Transport transport) const
    {
        std::string actions;
        for (uint8_t i = 0; i < _registeredActionsCount; i++)
        {
            if (isTransportEnabled(_actions[i], transport))
            {
                actions += _actions[i]->name + ", ";
            }
        }
        return actions;
    }

#if ENABLE_WEBSERVER
    void wireRestEndpoints();
#endif
};

extern ActionRegistry *actionRegistryInstance;
