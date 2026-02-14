#pragma once

#include "../../../config.h"

#if ENABLE_BERRY

#include "../UI/App.h"
#include "../UI/elements/container.h"
#include "../UI/Renderer.h"
#include "BerryUIBindings.h"
#include "../Logging.h"
#include "../../../fs/VirtualFS.h"
#include <vector>

extern "C" {
#include "berry.h"
}

enum class HandleType
{
    CONTAINER,
    LABEL,
    BUTTON,
    SCROLLABLE,
    CANVAS,
    CHECKBOX,
    RADIO,
    TEXTFIELD,
    COMBOBOX,
    GROUPBOX,
    TABS,
    FILELIST,
    MENUBAR
};

struct HandleEntry
{
    UI::Element *ptr;
    HandleType type;
};

class BerryApp : public UI::App
{
public:
    BerryApp(const String &scriptPath, const String &appName)
        : _scriptPath(scriptPath), _name(appName) {}

    const char *name() const override { return _name.c_str(); }

    void setup(UI::Container &content, int w, int h) override
    {
        bvm *vm = getBerryVM();
        if (!vm)
            return;

        berrySetCurrentApp(this);

        // handle 0 = content container
        _handles.clear();
        _handles.push_back({&content, HandleType::CONTAINER});

        // read script (resolve virtual path prefix)
        ResolvedPath resolved = resolveVirtualPath(_scriptPath);
        File f;
        if (resolved.valid && resolved.fs)
        {
            f = resolved.fs->open(resolved.localPath, "r");
        }
        else
        {
            f = LittleFS.open(_scriptPath, "r");
        }
        if (!f)
        {
            LoggerInstance->Error("BerryApp: cannot open " + _scriptPath);
            berrySetCurrentApp(nullptr);
            return;
        }
        String code = f.readString();
        f.close();

        // compile
        int res = be_loadbuffer(vm, _scriptPath.c_str(), code.c_str(), code.length());
        if (res != 0)
        {
            LoggerInstance->Error("BerryApp compile: " + String(be_tostring(vm, -1)));
            be_pop(vm, 1);
            berrySetCurrentApp(nullptr);
            return;
        }

        // execute script body — should return a class
        res = be_pcall(vm, 0);
        if (res != 0)
        {
            LoggerInstance->Error("BerryApp exec: " + String(be_tostring(vm, -1)));
            be_pop(vm, 1);
            berrySetCurrentApp(nullptr);
            return;
        }

        if (!be_isclass(vm, -1))
        {
            LoggerInstance->Error("BerryApp: script must return a class");
            be_pop(vm, 1);
            berrySetCurrentApp(nullptr);
            return;
        }

        // instantiate the class: call it as constructor
        res = be_pcall(vm, 0);
        if (res != 0)
        {
            LoggerInstance->Error("BerryApp init: " + String(be_tostring(vm, -1)));
            be_pop(vm, 1);
            berrySetCurrentApp(nullptr);
            return;
        }

        // store instance as global to prevent GC
        _instanceGlobal = "_bapp_" + String((unsigned long)this, HEX);
        be_setglobal(vm, _instanceGlobal.c_str());
        be_pop(vm, 1);

        // call instance.setup(content_handle, w, h)
        callMethod("setup", [&](bvm *v)
                   {
            be_pushint(v, 0); // content handle
            be_pushint(v, w);
            be_pushint(v, h);
            return 3; });

        berrySetCurrentApp(nullptr);
    }

    void teardown() override
    {
        bvm *vm = getBerryVM();
        if (!vm || _instanceGlobal.isEmpty())
            return;

        berrySetCurrentApp(this);

        callMethod("teardown", nullptr);

        // clean up callback globals
        for (auto &name : _callbackGlobals)
        {
            be_pushnil(vm);
            be_setglobal(vm, name.c_str());
            be_pop(vm, 1);
        }
        _callbackGlobals.clear();

        // remove instance global
        be_pushnil(vm);
        be_setglobal(vm, _instanceGlobal.c_str());
        be_pop(vm, 1);
        _instanceGlobal = "";

        _handles.clear();
        _nextCbId = 0;

        berrySetCurrentApp(nullptr);
    }

    // --- Handle table ---

    int addHandle(UI::Element *el, HandleType type)
    {
        int h = (int)_handles.size();
        _handles.push_back({el, type});
        return h;
    }

    HandleEntry *getHandle(int h)
    {
        if (h < 0 || h >= (int)_handles.size())
            return nullptr;
        if (!_handles[h].ptr)
            return nullptr;
        return &_handles[h];
    }

    void invalidateHandle(int h)
    {
        if (h >= 0 && h < (int)_handles.size())
            _handles[h].ptr = nullptr;
    }

    // --- Callback storage ---

    int storeCallback(bvm *vm, int stackIdx)
    {
        int id = _nextCbId++;
        String name = "_cb_" + String((unsigned long)this, HEX) + "_" + String(id);
        be_pushvalue(vm, stackIdx);
        be_setglobal(vm, name.c_str());
        be_pop(vm, 1);
        _callbackGlobals.push_back(name);
        return id;
    }

    void callBerryCallback(int cbId)
    {
        callBerryCallbackWithArgs(cbId, nullptr);
    }

    void callBerryCallbackWithArgs(int cbId, std::function<int(bvm *)> pushArgs)
    {
        bvm *vm = getBerryVM();
        if (!vm)
            return;

        String name = "_cb_" + String((unsigned long)this, HEX) + "_" + String(cbId);

        BerryApp *prev = berryCurrentApp();
        berrySetCurrentApp(this);

        be_getglobal(vm, name.c_str());
        if (be_isnil(vm, -1))
        {
            be_pop(vm, 1);
            berrySetCurrentApp(prev);
            return;
        }

        int argc = 0;
        if (pushArgs)
            argc = pushArgs(vm);

        int res = be_pcall(vm, argc);
        if (res != 0)
        {
            LoggerInstance->Error("BerryApp callback: " + String(be_tostring(vm, -1)));
        }
        be_pop(vm, argc + 1);

        berrySetCurrentApp(prev);
    }

    // --- Timer support ---

    void addBerryTimer(uint32_t ms, int cbId)
    {
        scheduleTimer(ms, [this, cbId]()
                      { callBerryCallback(cbId); });
    }

private:
    String _scriptPath;
    String _name;
    String _instanceGlobal;
    std::vector<HandleEntry> _handles;
    std::vector<String> _callbackGlobals;
    int _nextCbId{0};

    void callMethod(const char *method, std::function<int(bvm *)> pushArgs)
    {
        bvm *vm = getBerryVM();
        if (!vm || _instanceGlobal.isEmpty())
            return;

        be_getglobal(vm, _instanceGlobal.c_str());
        if (!be_isinstance(vm, -1))
        {
            be_pop(vm, 1);
            return;
        }

        be_getmethod(vm, -1, method);
        if (be_isnil(vm, -1))
        {
            be_pop(vm, 2);
            return;
        }

        // push self
        be_pushvalue(vm, -2);

        int argc = 1; // self
        if (pushArgs)
            argc += pushArgs(vm);

        int res = be_pcall(vm, argc);
        if (res != 0)
        {
            LoggerInstance->Error("BerryApp " + String(method) + ": " + String(be_tostring(vm, -1)));
        }
        be_pop(vm, argc + 2); // method + args + result + instance
    }
};

#endif // ENABLE_BERRY
