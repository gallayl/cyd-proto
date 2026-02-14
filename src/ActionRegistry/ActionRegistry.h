#pragma once

#include <Arduino.h>
#include "../config.h"
#include "FeatureAction.h"
#include "../CommandInterpreter/CommandParser.h"

#define ACTIONS_SIZE 24

class ActionRegistry
{
private:
    uint8_t _registeredActionsCount = 0;
    FeatureAction *_actions[ACTIONS_SIZE] = {};

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
    void RegisterAction(FeatureAction *action)
    {
        if (_registeredActionsCount >= ACTIONS_SIZE)
        {
            Serial.println(F("Action registry full, cannot register"));
            return;
        }
        _actions[_registeredActionsCount] = action;
        _registeredActionsCount++;
    }

    String Execute(const String &command, Transport transport)
    {
        for (uint8_t i = 0; i < _registeredActionsCount; i++)
        {
            const String &name = _actions[i]->name;
            if (command.equals(name) || command.startsWith(name + " "))
            {
                if (!isTransportEnabled(_actions[i], transport))
                {
                    return String("{\"error\": \"Action '" + name + "' not available on this transport\"}");
                }
                return _actions[i]->handler(command);
            }
        }
        return String("{\"message\": \"Unknown action: " + CommandParser::GetCommandName(command) + ".\", \"availableActions\": \"" + GetAvailableActions(transport) + "\"}");
    }

    String GetAvailableActions(Transport transport) const
    {
        String actions = "";
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
    void WireRestEndpoints();
#endif
};

extern ActionRegistry *ActionRegistryInstance;
