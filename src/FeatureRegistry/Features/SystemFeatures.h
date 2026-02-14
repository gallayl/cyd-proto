#pragma once

#include "../../config.h"
#include "AsyncJson.h"
#include "../registeredFeatures.h"
#include "../Feature.h"
#if ENABLE_WIFI
#include "./wifiCommand.h"
#endif
#include "./infoCustomCommand.h"
#include "./rgbLedCommand.h"
#include "./getLightSensorValueCommand.h"
#include "./getHallSensorValueCommand.h"
#include "../../CommandInterpreter/CommandInterpreter.h"
#include "../../CommandInterpreter/CustomCommand.h"
#include "../FeatureRegistry.h"

#if ENABLE_WEBSERVER
#include "../../services/WebServer.h"
#endif

// commands and handlers defined in SystemFeatures.cpp
extern CustomCommand *resetCommand;
extern CustomCommand *getRegisteredFeatures;

#if ENABLE_WEBSERVER
extern ArRequestHandlerFunction getFeaturesAction;
extern ArRequestHandlerFunction reset;
extern ArRequestHandlerFunction getInfoAction;
#endif

// feature object
extern Feature *SystemFeatures;
