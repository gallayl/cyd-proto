#pragma once

#include <WiFi.h>
#include <IPAddress.h>

#include "../../hw/WiFi.h"

#include "../../CommandInterpreter/CommandParser.h"
#include "../../CommandInterpreter/CustomCommand.h"

extern CustomCommand *wifiCommand;
