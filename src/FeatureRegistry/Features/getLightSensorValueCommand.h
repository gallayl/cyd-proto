
#include "../../CommandInterpreter/CommandParser.h"
#include "../../CommandInterpreter/CustomCommand.h"
#include "../../hw/lightSensor.h"
#include "./Logging.h"

CustomCommand *getLightSensorValueCommand = new CustomCommand("getLightSensorValue", [](String command)
                                                              { 
                                                                uint16_t value = readLightSensor();
                                                                LoggerInstance->Info("Read light sensor value: " + String(value));

                                                                return String("{\"event\": \"getLightSensorValue\", \"value\": " + String(value) + "}"); });