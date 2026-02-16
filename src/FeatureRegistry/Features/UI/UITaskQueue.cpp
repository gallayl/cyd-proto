#include "UITaskQueue.h"

#if ENABLE_UI

#include <freertos/queue.h>
#include <freertos/semphr.h>

namespace UI
{

struct UICommand
{
    std::function<void()> *action;
    SemaphoreHandle_t done;
};

static QueueHandle_t sCmdQueue = nullptr;
static TaskHandle_t sUiTaskHandle = nullptr;

static SemaphoreHandle_t sSyncMutex = nullptr;
static SemaphoreHandle_t sSyncDone = nullptr;

void initTaskQueue()
{
    sCmdQueue = xQueueCreate(8, sizeof(UICommand));
    sSyncMutex = xSemaphoreCreateMutex();
    sSyncDone = xSemaphoreCreateBinary();
}

void setUITaskHandle(TaskHandle_t handle)
{
    sUiTaskHandle = handle;
}

bool isUITaskRunning()
{
    return sUiTaskHandle != nullptr;
}

bool isOnUITask()
{
    return sUiTaskHandle != nullptr && xTaskGetCurrentTaskHandle() == sUiTaskHandle;
}

void processTaskQueue()
{
    if (sCmdQueue == nullptr)
    {
        return;
    }
    UICommand cmd;
    while (xQueueReceive(sCmdQueue, &cmd, 0) == pdTRUE)
    {
        if (cmd.action != nullptr)
        {
            (*cmd.action)();
        }
        if (cmd.done != nullptr)
        {
            xSemaphoreGive(cmd.done);
        }
    }
}

void postToUITaskSync(std::function<void()> action)
{
    if (isOnUITask() || !isUITaskRunning())
    {
        action();
        return;
    }
    xSemaphoreTake(sSyncMutex, portMAX_DELAY);
    UICommand cmd = {&action, sSyncDone};
    xQueueSend(sCmdQueue, &cmd, portMAX_DELAY);
    xSemaphoreTake(sSyncDone, portMAX_DELAY);
    xSemaphoreGive(sSyncMutex);
}

std::string postToUITaskWithResult(std::function<std::string()> action)
{
    std::string result;
    postToUITaskSync([&]() { result = action(); });
    return result;
}

} // namespace UI

#endif // ENABLE_UI
