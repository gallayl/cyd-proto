#pragma once

#include <cstdint>
#include <esp_timer.h>
#include <functional>
#include <vector>
#include <memory>
#include <LovyanGFX.hpp>
#include "elements/container.h"

namespace UI
{

class App
{
public:
    virtual ~App()
    {
        clearTimers();
    }
    virtual const char *name() const = 0;
    virtual void setup(Container &content, int w, int h) = 0;
    virtual void teardown() {}
    virtual bool hasIcon() const { return false; }
    virtual void drawIcon(LGFX_Sprite &canvas, int x, int y, int size) { (void)canvas; (void)x; (void)y; (void)size; }

    void scheduleTimer(uint32_t intervalMs, std::function<void()> cb)
    {
        auto t = std::make_unique<Timer>();
        t->fired = false;
        t->callback = std::move(cb);

        esp_timer_create_args_t args = {};
        args.callback = [](void *arg)
        {
            static_cast<Timer *>(arg)->fired = true;
        };
        args.arg = t.get();
        args.name = "app_timer";
        esp_timer_create(&args, &t->handle);
        esp_timer_start_periodic(t->handle, static_cast<uint64_t>(intervalMs) * 1000ULL);

        _timers.push_back(std::move(t));
    }

    bool tickTimers()
    {
        bool fired = false;
        for (auto &t : _timers)
        {
            if (t->fired)
            {
                t->fired = false;
                t->callback();
                fired = true;
            }
        }
        return fired;
    }

    void clearTimers()
    {
        for (auto &t : _timers)
        {
            esp_timer_stop(t->handle);
            esp_timer_delete(t->handle);
        }
        _timers.clear();
    }

private:
    struct Timer
    {
        esp_timer_handle_t handle = nullptr;
        volatile bool fired = false;
        std::function<void()> callback;
    };
    std::vector<std::unique_ptr<Timer>> _timers;
};

struct AppEntry
{
    const char *name;
    std::function<App *()> factory;
};

inline std::vector<AppEntry> &appRegistry()
{
    static std::vector<AppEntry> reg;
    return reg;
}

inline void registerApp(const char *name, std::function<App *()> factory)
{
    appRegistry().push_back({name, std::move(factory)});
}

} // namespace UI
