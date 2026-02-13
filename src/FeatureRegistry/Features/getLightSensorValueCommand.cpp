#include "getLightSensorValueCommand.h"

CustomCommand *getLightSensorValueCommand = new CustomCommand("getLightSensorValue", [](const String &command)
                                                              { 
                                                                uint16_t value = readLightSensor();
                                                                LoggerInstance->Info("Read light sensor value: " + String(value));

                                                                return String("{\"event\": \"getLightSensorValue\", \"value\": " + String(value) + "}"); });