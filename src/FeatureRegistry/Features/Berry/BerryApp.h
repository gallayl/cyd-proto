#pragma once

#include "../../../config.h"

#if ENABLE_BERRY

#include "../UI/App.h"
#include "../UI/elements/container.h"
#include "../UI/elements/error_popup.h"
#include "../UI/WindowManager.h"

#include "BerryUIBindings.h"
#include "../Logging.h"
#include "../../../fs/VirtualFS.h"
#include "../UI/BuiltinIcons.h"
#include "../../../utils/StringUtil.h"
#include <vector>
#include <string>

extern "C"
{
#include "berry.h"
}

enum class HandleType : uint8_t
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
    POPUP,
    ICON
};

struct HandleEntry
{
    UI::Element *ptr;
    HandleType type;
};

class BerryApp : public UI::App
{
public:
    BerryApp(const std::string &scriptPath, const std::string &appName, const std::string &iconType = "",
             const std::string &iconValue = "", const std::string &startMenu = "")
        : _scriptPath(scriptPath), _name(appName), _iconType(iconType), _iconValue(iconValue), _startMenu(startMenu)
    {
    }

    const char *name() const override
    {
        return _name.c_str();
    }

    const std::string &getIconType() const
    {
        return _iconType;
    }
    const std::string &getIconValue() const
    {
        return _iconValue;
    }

    bool hasIcon() const override
    {
        return true;
    }

    void drawIcon(LGFX_Sprite &canvas, int x, int y, int size) override
    {
        if (_iconType == "procedural")
        {
            // TODO: implement procedural icon via Berry VM callback (icon() method on instance)
            const char *fallback =
                !_startMenu.empty() ? UI::getDefaultIconForCategory(_startMenu.c_str()) : "generic_file";
            UI::drawBuiltinIcon(canvas, fallback, x, y, size);
        }
        else if (_iconType == "builtin" && !_iconValue.empty())
        {
            UI::drawBuiltinIcon(canvas, _iconValue.c_str(), x, y, size);
        }
        else if (_iconType.empty() && !_startMenu.empty())
        {
            const char *defaultIcon = UI::getDefaultIconForCategory(_startMenu.c_str());
            UI::drawBuiltinIcon(canvas, defaultIcon, x, y, size);
        }
        else
        {
            UI::drawBuiltinIcon(canvas, "generic_file", x, y, size);
        }
    }

    void setup(UI::Container &content, int w, int h) override
    {
        bvm *vm = getBerryVM();
        if (vm == nullptr)
        {
            return;
        }

        berrySetCurrentApp(this);

        // handle 0 = content container
        _handles.clear();
        _handles.push_back({&content, HandleType::CONTAINER});

        // read script (resolve virtual path prefix)
        ResolvedPath resolved = resolveVirtualPath(_scriptPath);
        File f;
        if (resolved.valid && resolved.fs != nullptr)
        {
            f = resolved.fs->open(resolved.localPath.c_str(), "r");
        }
        else
        {
            f = LittleFS.open(_scriptPath.c_str(), "r");
        }
        if (!f)
        {
            loggerInstance->Error(std::string("BerryApp: cannot open ") + _scriptPath);
            UI::errorPopup().show((std::string("Cannot open ") + _scriptPath).c_str());
            berrySetCurrentApp(nullptr);
            return;
        }
        std::string code = f.readString().c_str();
        f.close();

        // compile
        int res = be_loadbuffer(vm, _scriptPath.c_str(), code.c_str(), code.length());
        if (res != 0)
        {
            std::string err = be_tostring(vm, -1);
            loggerInstance->Error(std::string("BerryApp compile: ") + err);
            UI::errorPopup().show((std::string("Compile error:\n") + err).c_str());
            be_pop(vm, 1);
            berrySetCurrentApp(nullptr);
            return;
        }

        // execute script body — should return a class
        res = be_pcall(vm, 0);
        if (res != 0)
        {
            std::string err = be_tostring(vm, -1);
            loggerInstance->Error(std::string("BerryApp exec: ") + err);
            UI::errorPopup().show((std::string("Script error:\n") + err).c_str());
            be_pop(vm, 1);
            berrySetCurrentApp(nullptr);
            return;
        }

        if (!be_isclass(vm, -1))
        {
            loggerInstance->Error("BerryApp: script must return a class");
            UI::errorPopup().show("Script must return a class");
            be_pop(vm, 1);
            berrySetCurrentApp(nullptr);
            return;
        }

        // instantiate the class: call it as constructor
        res = be_pcall(vm, 0);
        if (res != 0)
        {
            std::string err = be_tostring(vm, -1);
            loggerInstance->Error(std::string("BerryApp init: ") + err);
            UI::errorPopup().show((std::string("Init error:\n") + err).c_str());
            be_pop(vm, 1);
            berrySetCurrentApp(nullptr);
            return;
        }

        // store instance as global to prevent GC
        _instanceGlobal = "_bapp_" + StringUtil::toHex((unsigned long)this);
        be_setglobal(vm, _instanceGlobal.c_str());
        be_pop(vm, 1);

        // call instance.setup(content_handle, w, h)
        callMethod("setup",
                   [&](bvm *v)
                   {
                       be_pushint(v, 0); // content handle
                       be_pushint(v, w);
                       be_pushint(v, h);
                       return 3;
                   });

        berrySetCurrentApp(nullptr);
    }

    void teardown() override
    {
        bvm *vm = getBerryVM();
        if (vm == nullptr || _instanceGlobal.empty())
        {
            return;
        }

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

        // destroy any popup overlays created by this app
        UI::windowManager().destroyPopupsForOwner(this);

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
        {
            return nullptr;
        }
        if (_handles[h].ptr == nullptr)
        {
            return nullptr;
        }
        return &_handles[h];
    }

    void invalidateHandle(int h)
    {
        if (h >= 0 && h < (int)_handles.size())
        {
            _handles[h].ptr = nullptr;
        }
    }

    // --- Callback storage ---

    int storeCallback(bvm *vm, int stackIdx)
    {
        int id = _nextCbId++;
        std::string name = "_cb_" + StringUtil::toHex((unsigned long)this) + "_" + std::to_string(id);
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

    void callBerryCallbackWithArgs(int cbId, const std::function<int(bvm *)> &pushArgs)
    {
        bvm *vm = getBerryVM();
        if (vm == nullptr)
        {
            return;
        }

        std::string name = "_cb_" + StringUtil::toHex((unsigned long)this) + "_" + std::to_string(cbId);

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
        {
            argc = pushArgs(vm);
        }

        int res = be_pcall(vm, argc);
        if (res != 0)
        {
            loggerInstance->Error(std::string("BerryApp callback: ") + be_tostring(vm, -1));
        }
        be_pop(vm, argc + 1);

        berrySetCurrentApp(prev);
    }

    // --- Timer support ---

    void addBerryTimer(uint32_t ms, int cbId)
    {
        scheduleTimer(ms, [this, cbId]() { callBerryCallback(cbId); });
    }

private:
    std::string _scriptPath;
    std::string _name;
    std::string _iconType;
    std::string _iconValue;
    std::string _startMenu;
    std::string _instanceGlobal;
    std::vector<HandleEntry> _handles;
    std::vector<std::string> _callbackGlobals;
    int _nextCbId{0};

    void callMethod(const char *method, const std::function<int(bvm *)> &pushArgs)
    {
        bvm *vm = getBerryVM();
        if ((vm == nullptr) || _instanceGlobal.empty())
        {
            return;
        }

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
        {
            argc += pushArgs(vm);
        }

        int res = be_pcall(vm, argc);
        if (res != 0)
        {
            loggerInstance->Error(std::string("BerryApp ") + method + ": " + be_tostring(vm, -1));
        }
        be_pop(vm, argc + 2); // method + args + result + instance
    }
};

#endif // ENABLE_BERRY
