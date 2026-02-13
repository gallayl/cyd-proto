#pragma once
#include "../Feature.h"
#include "../../CommandInterpreter/CommandInterpreter.h"

Feature *SerialReadFeature = new Feature("SerialRead", []()
                                         { return FeatureState::RUNNING; }, []()
                                         {
    if (Serial.available())
    {
        String command = Serial.readStringUntil('\n');
        command.replace("\n", "");
        command.replace("\r", "");
        String response = CommandInterpreterInstance->ExecuteCommand(command);
        Serial.println(response);
    } });
