#include "SerialRead.h"
#include <Arduino.h>
#include "../../ActionRegistry/ActionRegistry.h"

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
            String command(buf);
            command.replace("\r", "");
            String response = actionRegistryInstance->execute(command, Transport::CLI);
            Serial.println(response);
        }
    });
