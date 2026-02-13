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
        virtual void loop() {}
        virtual void teardown() {}
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
