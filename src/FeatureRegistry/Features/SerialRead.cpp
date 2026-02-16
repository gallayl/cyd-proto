#include "SerialRead.h"
#include <Arduino.h>
#include <string>
#include "../../ActionRegistry/ActionRegistry.h"
#include "../../utils/StringUtil.h"

static constexpr size_t MAX_SERIAL_INPUT = 256;

Feature *serialReadFeature = new Feature(
    "SerialRead", []() -> FeatureState { return FeatureState::RUNNING; },
    []()
    {
        if (Serial.available())
        {
            char buf[MAX_SERIAL_INPUT];
            size_t len = Serial.readBytesUntil('\n', buf, MAX_SERIAL_INPUT - 1);
            buf[len] = '\0';
            std::string command(buf);
            StringUtil::replaceAll(command, "\r", "");
            std::string response = actionRegistryInstance->execute(command, Transport::CLI);
            Serial.println(response.c_str());
        }
    });
