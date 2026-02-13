#include "getHallSensorValueCommand.h"

CustomCommand *getHallSensorValueCommand = new CustomCommand("getHallSensorValue", [](String command)
                                                             { 
                                                                uint16_t value = hallRead();
                                                                LoggerInstance->Info("Read hall sensor value: " + String(value));

                                                                return String("{\"event\": \"getHallSensorValue\", \"value\": " + String(value) + "}"); });