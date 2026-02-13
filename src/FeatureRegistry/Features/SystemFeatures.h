#pragma once

#include "AsyncJson.h"
#include "../registeredFeatures.h"
#include "../Feature.h"
#include "./wifiCommand.h"
#include "./infoCustomCommand.h"
#include "./rgbLedCommand.h"
#include "./getLightSensorValueCommand.h"
#include "./getHallSensorValueCommand.h"
#include "../../CommandInterpreter/CommandInterpreter.h"
#include "../../CommandInterpreter/CustomCommand.h"
#include "../FeatureRegistry.h"
#include "../../services/WebServer.h"

// commands and handlers defined in SystemFeatures.cpp
extern CustomCommand *resetCommand;
extern CustomCommand *getRegisteredFeatures;

extern ArRequestHandlerFunction getFeaturesAction;
extern ArRequestHandlerFunction reset;
extern ArRequestHandlerFunction getInfoAction;

// feature object
extern Feature *SystemFeatures;