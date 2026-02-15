#include "BerryFeature.h"
#include "../Logging.h"
#include "../../../ActionRegistry/ActionRegistry.h"
#include "../../../CommandInterpreter/CommandParser.h"
#include "../../../config.h"
#include "../../../fs/VirtualFS.h"
#include <LittleFS.h>

extern "C" {
#include "berry.h"
}

#if ENABLE_BERRY
#include "BerryUIBindings.h"
#include "BerryApp.h"
#include "../UI/App.h"
#if ENABLE_UI
#include "../UI/WindowManager.h"
#include "../UI/Theme.h"
#endif
#endif

static bvm *berry_vm = nullptr;

#if ENABLE_BERRY
bvm *getBerryVM() { return berry_vm; }

// Parse app name from "# app: <Name>" in first 5 lines of a .be file
static String parseAppNameFromFile(const String &path)
{
    ResolvedPath resolved = resolveVirtualPath(path);
    File f;
    if (resolved.valid && resolved.fs)
        f = resolved.fs->open(resolved.localPath, "r");
    else
        f = LittleFS.open(path, "r");
    if (!f)
        return String();

    String appName;
    for (int line = 0; line < 5 && f.available(); line++)
    {
        String l = f.readStringUntil('\n');
        l.trim();
        if (l.startsWith("# app:"))
        {
            appName = l.substring(6);
            appName.trim();
            break;
        }
    }
    f.close();

    if (appName.isEmpty())
    {
        int lastSlash = path.lastIndexOf('/');
        String filename = (lastSlash >= 0) ? path.substring(lastSlash + 1) : path;
        if (filename.endsWith(".be"))
            filename = filename.substring(0, filename.length() - 3);
        if (filename.length() > 0)
            filename.setCharAt(0, toupper(filename.charAt(0)));
        appName = filename;
    }

    return appName;
}

#if ENABLE_UI
void openBerryScript(const String &filePath)
{
    String path = filePath;
    if (path.length() > 0 && path[0] != '/')
        path = "/" + path;

    String appName = parseAppNameFromFile(path);
    if (appName.isEmpty())
        return;

    auto *app = new BerryApp(path, appName);
    UI::windowManager().openApp(appName.c_str(), app);
}

void openBerryPanel(const String &filePath)
{
    String path = filePath;
    if (path.length() > 0 && path[0] != '/')
        path = "/" + path;

    String appName = parseAppNameFromFile(path);
    if (appName.isEmpty())
        return;

    auto *app = new BerryApp(path, appName);
    UI::windowManager().openPanel(
        appName.c_str(), app,
        0, UI::Theme::TaskbarY, UI::Theme::ScreenWidth, UI::Theme::TaskbarHeight);
}
#endif

static std::vector<BerryScriptInfo> scanBerryScriptsOnFs(fs::FS &filesystem, const char *localDir, const String &virtualPrefix)
{
    std::vector<BerryScriptInfo> result;

    if (!filesystem.exists(localDir))
        return result;

    File d = filesystem.open(localDir);
    if (!d || !d.isDirectory())
        return result;

    File entry = d.openNextFile();
    while (entry)
    {
        String filename = entry.name();
        if (filename.endsWith(".be"))
        {
            String localPath = String(localDir);
            if (!localPath.endsWith("/"))
                localPath += "/";
            localPath += filename;

            String virtualPath = virtualPrefix + localPath;

            BerryScriptInfo info;
            info.name = parseAppNameFromFile(virtualPath);
            info.path = virtualPath;
            result.push_back(std::move(info));
        }
        entry = d.openNextFile();
    }
    return result;
}

std::vector<BerryScriptInfo> scanBerryScripts(const char *dir)
{
    // Scan on LittleFS (flash)
    auto result = scanBerryScriptsOnFs(LittleFS, dir, "/flash");

#if ENABLE_SD_CARD
    // Also scan on SD card if mounted
    if (isSdMounted())
    {
        auto sdResult = scanBerryScriptsOnFs(SD, dir, "/sd");
        result.insert(result.end(), sdResult.begin(), sdResult.end());
    }
#endif

    return result;
}
#endif

// --- Native bindings exposed to Berry scripts ---

static int native_action(bvm *vm)
{
    int argc = be_top(vm);
    if (argc >= 1 && be_isstring(vm, 1))
    {
        const char *cmd = be_tostring(vm, 1);
        String result = ActionRegistryInstance->Execute(String(cmd), Transport::SCRIPTING);
        be_pushstring(vm, result.c_str());
        be_return(vm);
    }
    be_return_nil(vm);
}

static int native_log(bvm *vm)
{
    int argc = be_top(vm);
    if (argc >= 1 && be_isstring(vm, 1))
    {
        const char *msg = be_tostring(vm, 1);
        LoggerInstance->Info(String(msg));
    }
    be_return_nil(vm);
}

// --- Berry eval/run helpers ---

static String berryEval(const String &code)
{
    if (!berry_vm)
    {
        return String(F("{\"error\": \"Berry VM not initialized\"}"));
    }

    int result = be_loadbuffer(berry_vm, "input", code.c_str(), code.length());
    if (result == 0)
    {
        result = be_pcall(berry_vm, 0);
    }

    String output;
    if (result != 0)
    {
        const char *err = be_tostring(berry_vm, -1);
        output = "{\"error\": \"" + String(err) + "\"}";
        LoggerInstance->Error("Berry: " + String(err));
        be_pop(berry_vm, 1);
    }
    else
    {
        if (be_isstring(berry_vm, -1))
        {
            output = "{\"result\": \"" + String(be_tostring(berry_vm, -1)) + "\"}";
        }
        else if (be_isint(berry_vm, -1))
        {
            output = "{\"result\": " + String((long)be_toint(berry_vm, -1)) + "}";
        }
        else if (be_isreal(berry_vm, -1))
        {
            output = "{\"result\": " + String(be_toreal(berry_vm, -1)) + "}";
        }
        else if (be_isbool(berry_vm, -1))
        {
            output = "{\"result\": " + String(be_tobool(berry_vm, -1) ? "true" : "false") + "}";
        }
        else
        {
            output = String(F("{\"result\": null}"));
        }
        be_pop(berry_vm, 1);
    }

    return output;
}

// --- Action handler ---

static String berryHandler(const String &command)
{
    String operation = CommandParser::GetCommandParameter(command, 1);

    if (operation == "eval")
    {
        // everything after "berry eval " is the code to evaluate
        int idx = command.indexOf("eval ");
        if (idx < 0)
        {
            return String(F("{\"error\": \"No code provided\"}"));
        }
        String code = command.substring(idx + 5);
        return berryEval(code);
    }

    if (operation == "run")
    {
        String path = CommandParser::GetCommandParameter(command, 2);
        if (path.length() == 0)
        {
            return String(F("{\"error\": \"No file path provided\"}"));
        }

        if (path.length() > 0 && path[0] != '/')
            path = "/" + path;

        // Resolve virtual path to the correct filesystem
        ResolvedPath resolved = resolveVirtualPath(path);
        bool fileExists = false;
        if (resolved.valid && resolved.fs)
        {
            fileExists = resolved.fs->exists(resolved.localPath);
        }
        else
        {
            // Legacy path without prefix — assume flash
            fileExists = LittleFS.exists(path);
            if (fileExists)
                path = "/flash" + path;
        }

        if (!fileExists)
        {
            return "{\"error\": \"File not found: " + path + "\"}";
        }

#if ENABLE_UI
        openBerryScript(path);
        String appName = parseAppNameFromFile(path);
        LoggerInstance->Info("Berry: opened " + appName + " (" + path + ")");
        return "{\"event\":\"berry\", \"status\":\"opened\", \"app\":\"" + appName + "\"}";
#else
        ResolvedPath rp = resolveVirtualPath(path);
        File f;
        if (rp.valid && rp.fs)
            f = rp.fs->open(rp.localPath, "r");
        else
            f = LittleFS.open(path, "r");
        if (!f)
            return "{\"error\": \"File not found: " + path + "\"}";
        String code = f.readString();
        f.close();
        return berryEval(code);
#endif
    }

#if ENABLE_UI
    if (operation == "open")
    {
        String appName = CommandParser::GetCommandParameter(command, 2);
        if (appName.length() == 0)
        {
            return String(F("{\"error\": \"No app name provided\"}"));
        }
        // scan /berry/apps/ and find matching name
        auto scripts = scanBerryScripts("/berry/apps");
        for (auto &s : scripts)
        {
            if (s.name.equalsIgnoreCase(appName))
            {
                openBerryScript(s.path);
                LoggerInstance->Info("Berry: opened app " + s.name);
                return "{\"event\":\"berry\", \"status\":\"opened\", \"app\":\"" + s.name + "\"}";
            }
        }
        return "{\"error\": \"Unknown Berry app: " + appName + "\"}";
    }

    if (operation == "panel")
    {
        String appName = CommandParser::GetCommandParameter(command, 2);
        if (appName.length() == 0)
        {
            return String(F("{\"error\": \"No app name provided\"}"));
        }
        auto scripts = scanBerryScripts("/berry/apps");
        for (auto &s : scripts)
        {
            if (s.name.equalsIgnoreCase(appName))
            {
                openBerryPanel(s.path);
                LoggerInstance->Info("Berry: opened panel " + s.name);
                return "{\"event\":\"berry\", \"status\":\"panel_opened\", \"app\":\"" + s.name + "\"}";
            }
        }
        return "{\"error\": \"Unknown Berry app: " + appName + "\"}";
    }
#endif

    return String(F("{\"error\": \"Usage: berry eval <code> | berry run <path> | berry open <appname> | berry panel <appname>\"}"));
}

// --- Action definition ---

static FeatureAction berryAction = {
    .name = "berry",
    .handler = berryHandler,
    .transports = {.cli = true, .rest = false, .ws = true, .scripting = true}};

// --- Feature ---

static void registerNativeFunction(const char *name, bntvfunc func)
{
    be_pushntvfunction(berry_vm, func);
    be_setglobal(berry_vm, name);
    be_pop(berry_vm, 1);
}

#if ENABLE_BERRY
#endif

Feature *BerryFeature = new Feature(
    "Berry",
    []()
    {
        berry_vm = be_vm_new();
        if (!berry_vm)
        {
            LoggerInstance->Error(F("Failed to create Berry VM"));
            return FeatureState::ERROR;
        }

        registerNativeFunction("action", native_action);
        registerNativeFunction("log", native_log);

#if ENABLE_BERRY
        registerBerryUIModule(berry_vm);
#endif

        ActionRegistryInstance->RegisterAction(&berryAction);

        if (LittleFS.exists("/berry/autoexec.be"))
        {
            LoggerInstance->Info(F("Running /berry/autoexec.be"));
            File f = LittleFS.open("/berry/autoexec.be", "r");
            if (f)
            {
                String code = f.readString();
                f.close();
                berryEval(code);
            }
        }

        LoggerInstance->Info(F("Berry VM initialized"));
        return FeatureState::RUNNING;
    },
    []() {},
    []()
    {
        if (berry_vm)
        {
            be_vm_delete(berry_vm);
            berry_vm = nullptr;
            LoggerInstance->Info(F("Berry VM destroyed"));
        }
    });
