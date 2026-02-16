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

static QueueHandle_t s_cmdQueue = nullptr;
static TaskHandle_t s_uiTaskHandle = nullptr;

static SemaphoreHandle_t s_syncMutex = nullptr;
static SemaphoreHandle_t s_syncDone = nullptr;

void initTaskQueue()
{
    s_cmdQueue = xQueueCreate(8, sizeof(UICommand));
    s_syncMutex = xSemaphoreCreateMutex();
    s_syncDone = xSemaphoreCreateBinary();
}

void setUITaskHandle(TaskHandle_t handle)
{
    s_uiTaskHandle = handle;
}

bool isUITaskRunning()
{
    return s_uiTaskHandle != nullptr;
}

bool isOnUITask()
{
    return s_uiTaskHandle != nullptr && xTaskGetCurrentTaskHandle() == s_uiTaskHandle;
}

void processTaskQueue()
{
    if (!s_cmdQueue)
        return;
    UICommand cmd;
    while (xQueueReceive(s_cmdQueue, &cmd, 0) == pdTRUE)
    {
        if (cmd.action)
        {
            (*cmd.action)();
        }
        if (cmd.done)
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
    xSemaphoreTake(s_syncMutex, portMAX_DELAY);
    UICommand cmd = {&action, s_syncDone};
    xQueueSend(s_cmdQueue, &cmd, portMAX_DELAY);
    xSemaphoreTake(s_syncDone, portMAX_DELAY);
    xSemaphoreGive(s_syncMutex);
}

String postToUITaskWithResult(std::function<String()> action)
{
    String result;
    postToUITaskSync([&]()
                     { result = action(); });
    return result;
}

} // namespace UI

#endif // ENABLE_UI
