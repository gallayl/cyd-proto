#pragma once

#include "../../../config.h"

#if ENABLE_UI

#include <functional>
#include <string>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace UI
{

void initTaskQueue();
void setUITaskHandle(TaskHandle_t handle);
bool isUITaskRunning();
bool isOnUITask();

void processTaskQueue();

void postToUITaskSync(std::function<void()> action);

std::string postToUITaskWithResult(std::function<std::string()> action);

} // namespace UI

#endif // ENABLE_UI
