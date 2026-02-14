#include "SerialRead.h"
#include <Arduino.h>

Feature *SerialReadFeature = new Feature("SerialRead", []() -> FeatureState
                                         { return FeatureState::RUNNING; }, []()
                                         {
                                            if (Serial.available())
                                            {
                                                String command = Serial.readStringUntil('\n');
                                                command.replace("\n", "");
                                                command.replace("\r", "");
                                                String response = ActionRegistryInstance->Execute(command, Transport::CLI);
                                                Serial.println(response);
                                            } });
