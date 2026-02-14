#include "BerryFeature.h"
#include "../Logging.h"
#include "../../../ActionRegistry/ActionRegistry.h"
#include "../../../CommandInterpreter/CommandParser.h"
#include <LittleFS.h>

extern "C" {
#include "berry.h"
}

#if ENABLE_BERRY
#include "BerryUIBindings.h"
#include "BerryApp.h"
#include "../UI/App.h"
#endif

static bvm *berry_vm = nullptr;

#if ENABLE_BERRY
bvm *getBerryVM() { return berry_vm; }

static std::vector<String> s_berryAppNames;
const std::vector<String> &getBerryAppNames() { return s_berryAppNames; }
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

        File f = LittleFS.open(path, "r");
        if (!f)
        {
            return "{\"error\": \"File not found: " + path + "\"}";
        }
        String code = f.readString();
        f.close();

        return berryEval(code);
    }

    return String(F("{\"error\": \"Usage: berry eval <code> | berry run <path>\"}"));
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
// --- App discovery ---

static void discoverBerryApps()
{
    s_berryAppNames.clear();

    if (!LittleFS.exists("/berry/apps"))
        return;

    File dir = LittleFS.open("/berry/apps");
    if (!dir || !dir.isDirectory())
        return;

    File entry = dir.openNextFile();
    while (entry)
    {
        String filename = entry.name();
        if (filename.endsWith(".be"))
        {
            // Parse app name from "# app: <Name>" in first 5 lines
            String appName;
            String fullPath = "/berry/apps/" + filename;

            File f = LittleFS.open(fullPath, "r");
            if (f)
            {
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
            }

            // fallback to filename without extension
            if (appName.isEmpty())
            {
                appName = filename.substring(0, filename.length() - 3);
                // capitalize first letter
                if (appName.length() > 0)
                    appName.setCharAt(0, toupper(appName.charAt(0)));
            }

            // register app factory
            String path = fullPath;
            String name = appName;
            UI::registerApp(name.c_str(), [path, name]() -> UI::App *
                            { return new BerryApp(path, name); });
            s_berryAppNames.push_back(name);

            LoggerInstance->Info("Berry app: " + name + " (" + fullPath + ")");
        }
        entry = dir.openNextFile();
    }
}
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
        discoverBerryApps();
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
