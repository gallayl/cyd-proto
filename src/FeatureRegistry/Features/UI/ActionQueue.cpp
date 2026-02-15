#include "ActionQueue.h"
#include <vector>

namespace UI
{

static std::vector<std::function<void()>> g_queue;

void queueAction(std::function<void()> action)
{
    if (action)
        g_queue.push_back(std::move(action));
}

void executeQueuedActions()
{
    if (g_queue.empty())
        return;
    // move callbacks out so queue can be modified while running
    std::vector<std::function<void()>> toRun;
    toRun.swap(g_queue);
    for (auto &cb : toRun)
    {
        if (cb)
        {
            cb();
        }
    }
}

} // namespace UI
