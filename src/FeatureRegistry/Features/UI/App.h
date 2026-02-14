#pragma once

#include <Arduino.h>
#include <functional>
#include <vector>
#include "elements/container.h"

namespace UI
{

    class App
    {
    public:
        virtual ~App() = default;
        virtual const char *name() const = 0;
        virtual void setup(Container &content, int w, int h) = 0;
        virtual void teardown() {}

        void scheduleTimer(uint32_t intervalMs, std::function<void()> cb)
        {
            _timers.push_back({intervalMs, millis(), std::move(cb)});
        }

        // returns true if any timer fired
        bool tickTimers()
        {
            bool fired = false;
            unsigned long now = millis();
            for (auto &t : _timers)
            {
                if (now - t.lastTick >= t.intervalMs)
                {
                    t.callback();
                    t.lastTick = now;
                    fired = true;
                }
            }
            return fired;
        }

    private:
        struct Timer
        {
            uint32_t intervalMs;
            unsigned long lastTick;
            std::function<void()> callback;
        };
        std::vector<Timer> _timers;
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
