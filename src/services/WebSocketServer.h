#pragma once

#include "../config.h"

#if ENABLE_WEBSERVER

#include <string>

void initWebSockets();
void wsBroadcast(const std::string &msg);
void wsOnSessionClose(int sockfd);

#endif
