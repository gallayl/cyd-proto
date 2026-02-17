#include "BerryFeature.h"
#include "../Logging.h"
#include "../../../ActionRegistry/ActionRegistry.h"
#include "../../../CommandInterpreter/CommandParser.h"
#include "../../../config.h"
#include "../../../fs/VirtualFS.h"
#include "../../../utils/StringUtil.h"
#include <string>

#ifdef USE_ESP_IDF
#include <cstdio>
#include <dirent.h>
#include <sys/stat.h>
#else
#include <LittleFS.h>
#endif

extern "C"
{
#include "berry.h"
}

#if ENABLE_BERRY
#include "BerryUIBindings.h"
#include "BerryApp.h"
#if ENABLE_UI
#include "../UI/WindowManager.h"
#include "../UI/Theme.h"
#include "../UI/UITaskQueue.h"
#include "../UI/ActionQueue.h"
#endif
#endif

static bvm *berry_vm = nullptr;

#if ENABLE_BERRY
bvm *getBerryVM()
{
    return berry_vm;
}

BerryScriptInfo parseAppMetadata(const std::string &path)
{
    BerryScriptInfo info;
    info.path = path;

    ResolvedPath resolved = resolveVirtualPath(path);

#ifdef USE_ESP_IDF
    std::string realPath = resolved.valid ? resolved.realPath : resolveToLittleFsPath(path);

    FILE *f = fopen(realPath.c_str(), "r");
    if (!f)
    {
        return info;
    }

    char lineBuf[256];
    for (int line = 0; line < 15 && fgets(lineBuf, sizeof(lineBuf), f) != nullptr; line++)
    {
        std::string l = StringUtil::trim(std::string(lineBuf));
#else
    File f;
    if (resolved.valid && resolved.fs != nullptr)
    {
        f = resolved.fs->open(resolved.localPath.c_str(), "r");
    }
    else
    {
        f = LittleFS.open(path.c_str(), "r");
    }
    if (!f)
    {
        return info;
    }

    for (int line = 0; line < 15 && (f.available() != 0); line++)
    {
        std::string l = f.readStringUntil('\n').c_str();
        l = StringUtil::trim(l);
#endif
        if (StringUtil::startsWith(l, "# app:"))
        {
            info.name = StringUtil::trim(l.substr(6));
        }
        else if (StringUtil::startsWith(l, "# startMenu:"))
        {
            info.startMenu = StringUtil::trim(l.substr(12));
        }
        else if (StringUtil::startsWith(l, "# icon:"))
        {
            std::string iconSpec = StringUtil::trim(l.substr(7));
            if (StringUtil::startsWith(iconSpec, "builtin/"))
            {
                info.iconType = "builtin";
                info.iconValue = iconSpec.substr(8);
            }
            else if (iconSpec == "procedural")
            {
                info.iconType = "procedural";
            }
            else if (StringUtil::startsWith(iconSpec, "/"))
            {
                info.iconType = "file";
                info.iconValue = iconSpec;
            }
        }
        if (!info.name.empty() && !info.startMenu.empty())
        {
            break;
        }
    }
#ifdef USE_ESP_IDF
    fclose(f);
#else
    f.close();
#endif

    if (info.name.empty())
    {
        size_t lastSlash = path.rfind('/');
        std::string filename = (lastSlash != std::string::npos) ? path.substr(lastSlash + 1) : path;
        if (StringUtil::endsWith(filename, ".be"))
        {
            filename = filename.substr(0, filename.length() - 3);
        }
        if (filename.length() > 0)
        {
            filename[0] = toupper(filename[0]);
        }
        info.name = filename;
    }

    return info;
}

#if ENABLE_UI
void openBerryScript(const std::string &filePath)
{
    std::string path = filePath;
    if (path.length() > 0 && path[0] != '/')
    {
        path = std::string("/") + path;
    }

    auto meta = parseAppMetadata(path);
    if (meta.name.empty())
    {
        return;
    }

    auto *app = new BerryApp(path, meta.name, meta.iconType, meta.iconValue, meta.startMenu);
    UI::windowManager().openApp(meta.name.c_str(), app);
}

void openBerryPanel(const std::string &filePath)
{
    std::string path = filePath;
    if (path.length() > 0 && path[0] != '/')
    {
        path = std::string("/") + path;
    }

    auto meta = parseAppMetadata(path);
    if (meta.name.empty())
    {
        return;
    }

    auto *app = new BerryApp(path, meta.name, meta.iconType, meta.iconValue, meta.startMenu);
    UI::windowManager().openPanel(meta.name.c_str(), app, 0, UI::Theme::TaskbarY(), UI::Theme::ScreenWidth(),
                                  UI::Theme::TaskbarHeight);
}
#endif

#ifdef USE_ESP_IDF

static std::vector<BerryScriptInfo> scanBerryScriptsOnFs(const std::string &realDir)
{
    std::vector<BerryScriptInfo> result;

    DIR *d = opendir(realDir.c_str());
    if (!d)
    {
        return result;
    }

    struct dirent *entry;
    while ((entry = readdir(d)) != nullptr)
    {
        std::string filename = entry->d_name;
        if (StringUtil::endsWith(filename, ".be"))
        {
            std::string realPath = realDir;
            if (!StringUtil::endsWith(realPath, "/"))
            {
                realPath += "/";
            }
            realPath += filename;

            std::string virtualPath = toVirtualPath(realPath);

            BerryScriptInfo info = parseAppMetadata(virtualPath);
            result.push_back(std::move(info));
        }
    }
    closedir(d);
    return result;
}

std::vector<BerryScriptInfo> scanBerryScripts(const char *dir)
{
    // Scan on LittleFS (flash)
    auto result = scanBerryScriptsOnFs(resolveToLittleFsPath(dir));

#if ENABLE_SD_CARD
    // Also scan on SD card if mounted
    if (isSdMounted())
    {
        auto sdResult = scanBerryScriptsOnFs(std::string(SD_MOUNT_POINT) + dir);
        result.insert(result.end(), sdResult.begin(), sdResult.end());
    }
#endif

    return result;
}

#else // Arduino

static std::vector<BerryScriptInfo> scanBerryScriptsOnFs(fs::FS &filesystem, const char *localDir,
                                                         const std::string &virtualPrefix)
{
    std::vector<BerryScriptInfo> result;

    if (!filesystem.exists(localDir))
    {
        return result;
    }

    File d = filesystem.open(localDir);
    if (!d || !d.isDirectory())
    {
        return result;
    }

    File entry = d.openNextFile();
    while (entry)
    {
        std::string filename = entry.name();
        if (StringUtil::endsWith(filename, ".be"))
        {
            std::string localPath = localDir;
            if (!StringUtil::endsWith(localPath, "/"))
            {
                localPath += "/";
            }
            localPath += filename;

            std::string virtualPath = virtualPrefix + localPath;

            BerryScriptInfo info = parseAppMetadata(virtualPath);
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

#endif // USE_ESP_IDF

#endif // ENABLE_BERRY

// --- Native bindings exposed to Berry scripts ---

static int native_action(bvm *vm)
{
    int argc = be_top(vm);
    if (argc >= 1 && be_isstring(vm, 1))
    {
        const char *cmd = be_tostring(vm, 1);
        std::string result = actionRegistryInstance->execute(std::string(cmd), Transport::SCRIPTING);
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
        loggerInstance->Info(std::string(msg));
    }
    be_return_nil(vm);
}

// --- Berry eval/run helpers ---

static std::string berryEval(const std::string &code)
{
    if (berry_vm == nullptr)
    {
        return "{\"error\": \"Berry VM not initialized\"}";
    }

    int result = be_loadbuffer(berry_vm, "input", code.c_str(), code.length());
    if (result == 0)
    {
        result = be_pcall(berry_vm, 0);
    }

    std::string output;
    if (result != 0)
    {
        const char *err = be_tostring(berry_vm, -1);
        output = std::string("{\"error\": \"") + err + "\"}";
        loggerInstance->Error(std::string("Berry: ") + err);
        be_pop(berry_vm, 1);
    }
    else
    {
        if (be_isstring(berry_vm, -1))
        {
            output = std::string("{\"result\": \"") + be_tostring(berry_vm, -1) + "\"}";
        }
        else if (be_isint(berry_vm, -1))
        {
            output = std::string("{\"result\": ") + std::to_string((long)be_toint(berry_vm, -1)) + "}";
        }
        else if (be_isreal(berry_vm, -1))
        {
            output = std::string("{\"result\": ") + std::to_string(be_toreal(berry_vm, -1)) + "}";
        }
        else if (be_isbool(berry_vm, -1))
        {
            output = std::string("{\"result\": ") + (be_tobool(berry_vm, -1) ? "true" : "false") + "}";
        }
        else
        {
            output = "{\"result\": null}";
        }
        be_pop(berry_vm, 1);
    }

    return output;
}

// --- Action handler ---

static std::string berryHandlerImpl(const std::string &command)
{
    std::string operation = CommandParser::getCommandParameter(command, 1);

    if (operation == "eval")
    {
        size_t idx = command.find("eval ");
        if (idx == std::string::npos)
        {
            return "{\"error\": \"No code provided\"}";
        }
        std::string code = command.substr(idx + 5);
        return berryEval(code);
    }

    if (operation == "run")
    {
        std::string path = CommandParser::getCommandParameter(command, 2);
        if (path.length() == 0)
        {
            return "{\"error\": \"No file path provided\"}";
        }

        if (path.length() > 0 && path[0] != '/')
        {
            path = std::string("/") + path;
        }

        ResolvedPath resolved = resolveVirtualPath(path);
        bool fileExists = false;
#ifdef USE_ESP_IDF
        if (resolved.valid)
        {
            fileExists = vfsExists(resolved.realPath);
        }
        else
        {
            std::string lfsPath = resolveToLittleFsPath(path);
            fileExists = vfsExists(lfsPath);
            if (fileExists)
            {
                path = std::string("/flash") + path;
            }
        }
#else
        if (resolved.valid && (resolved.fs != nullptr))
        {
            fileExists = resolved.fs->exists(resolved.localPath.c_str());
        }
        else
        {
            fileExists = LittleFS.exists(path.c_str());
            if (fileExists)
            {
                path = std::string("/flash") + path;
            }
        }
#endif

        if (!fileExists)
        {
            return std::string("{\"error\": \"File not found: ") + path + "\"}";
        }

#if ENABLE_UI
        {
            auto meta = parseAppMetadata(path);
            const std::string pathCopy = path;
            UI::queueAction([pathCopy]() { openBerryScript(pathCopy); });
            loggerInstance->Info(std::string("Berry: opening ") + meta.name + " (" + path + ")");
            return std::string(R"({"event":"berry", "status":"queued", "app":")") + meta.name + "\"}";
        }
#else
#ifdef USE_ESP_IDF
        ResolvedPath rp = resolveVirtualPath(path);
        std::string realPath = rp.valid ? rp.realPath : resolveToLittleFsPath(path);
        std::string code = vfsReadFileAsString(realPath);
        if (code.empty())
            return std::string("{\"error\": \"File not found: ") + path + "\"}";
        return berryEval(code);
#else
        ResolvedPath rp = resolveVirtualPath(path);
        File f;
        if (rp.valid && rp.fs)
            f = rp.fs->open(rp.localPath.c_str(), "r");
        else
            f = LittleFS.open(path.c_str(), "r");
        if (!f)
            return std::string("{\"error\": \"File not found: ") + path + "\"}";
        std::string code = f.readString().c_str();
        f.close();
        return berryEval(code);
#endif
#endif
    }

#if ENABLE_UI
    if (operation == "open")
    {
        std::string appName = CommandParser::getCommandParameter(command, 2);
        if (appName.length() == 0)
        {
            return "{\"error\": \"No app name provided\"}";
        }
        auto scripts = scanBerryScripts("/berry/apps");
        for (auto &s : scripts)
        {
            if (StringUtil::equalsIgnoreCase(s.name, appName))
            {
                std::string scriptPath = s.path;
                UI::queueAction([scriptPath]() { openBerryScript(scriptPath); });
                loggerInstance->Info(std::string("Berry: opening app ") + s.name);
                return std::string("{\"event\":\"berry\", \"status\":\"queued\", \"app\":\"") + s.name + "\"}";
            }
        }
        return std::string(R"({"error": "Unknown Berry app: )") + appName + "\"}";
    }

    if (operation == "panel")
    {
        std::string appName = CommandParser::getCommandParameter(command, 2);
        if (appName.length() == 0)
        {
            return "{\"error\": \"No app name provided\"}";
        }
        auto scripts = scanBerryScripts("/berry/apps");
        for (auto &s : scripts)
        {
            if (StringUtil::equalsIgnoreCase(s.name, appName))
            {
                openBerryPanel(s.path);
                loggerInstance->Info(std::string("Berry: opened panel ") + s.name);
                return std::string(R"({"event":"berry", "status":"panel_opened", "app":")") + s.name + "\"}";
            }
        }
        return std::string(R"({"error": "Unknown Berry app: )") + appName + "\"}";
    }
#endif

    if (operation == "apps")
    {
        auto scripts = scanBerryScripts("/berry/apps");
        std::string json = "{\"apps\":[";
        for (size_t i = 0; i < scripts.size(); i++)
        {
            if (i > 0)
            {
                json += ",";
            }

            // parse category and label from startMenu path (e.g. "/Programs/Paint")
            std::string category;
            std::string label;
            const std::string &sm = scripts[i].startMenu;
            if (sm.length() > 1 && sm[0] == '/')
            {
                size_t sep = sm.find('/', 1);
                if (sep != std::string::npos)
                {
                    category = sm.substr(1, sep - 1);
                    label = sm.substr(sep + 1);
                }
            }

            json += std::string("{\"name\":\"") + scripts[i].name + "\",\"path\":\"" + scripts[i].path +
                    "\",\"startMenu\":\"" + sm + "\",\"category\":\"" + category + "\",\"label\":\"" + label +
                    "\",\"iconType\":\"" + scripts[i].iconType + "\",\"iconValue\":\"" + scripts[i].iconValue + "\"}";
        }
        json += "]}";
        return json;
    }

    if (operation == "meta")
    {
        std::string path = CommandParser::getCommandParameter(command, 2);
        if (path.length() == 0)
        {
            return "{\"error\": \"No file path provided\"}";
        }
        if (path.length() > 0 && path[0] != '/')
        {
            path = std::string("/") + path;
        }
        auto meta = parseAppMetadata(path);
        return std::string(R"({"app":")") + meta.name + "\",\"startMenu\":\"" + meta.startMenu + "\",\"iconType\":\"" +
               meta.iconType + "\",\"iconValue\":\"" + meta.iconValue + "\"}";
    }

    return "{\"error\": \"Usage: berry eval <code> | berry run <path> | berry open <appname> | berry panel "
           "<appname> | berry apps | berry meta <path>\"}";
}

static std::string berryHandler(const std::string &command)
{
#if ENABLE_UI
    return UI::postToUITaskWithResult([&command]() -> std::string { return berryHandlerImpl(command); });
#else
    return berryHandlerImpl(command);
#endif
}

// --- Action definition ---

static FeatureAction berryAction = {.name = "berry",
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
            loggerInstance->Error("Failed to create Berry VM");
            return FeatureState::ERROR;
        }

        registerNativeFunction("action", native_action);
        registerNativeFunction("log", native_log);

#if ENABLE_BERRY
        registerBerryUIModule(berry_vm);
#endif

        actionRegistryInstance->registerAction(&berryAction);

#ifdef USE_ESP_IDF
        std::string autoexecPath = resolveToLittleFsPath("/berry/autoexec.be");
        if (vfsExists(autoexecPath))
        {
            loggerInstance->Info("Running /berry/autoexec.be");
            std::string code = vfsReadFileAsString(autoexecPath);
            if (!code.empty())
            {
                berryEval(code);
            }
        }
#else
        if (LittleFS.exists("/berry/autoexec.be"))
        {
            loggerInstance->Info("Running /berry/autoexec.be");
            File f = LittleFS.open("/berry/autoexec.be", "r");
            if (f)
            {
                std::string code = f.readString().c_str();
                f.close();
                berryEval(code);
            }
        }
#endif

        loggerInstance->Info("Berry VM initialized");
        return FeatureState::RUNNING;
    },
    []() {},
    []()
    {
        if (berry_vm)
        {
            auto doDelete = []()
            {
                be_vm_delete(berry_vm);
                berry_vm = nullptr;
            };
#if ENABLE_UI
            if (UI::isUITaskRunning())
            {
                UI::postToUITaskSync(doDelete);
            }
            else
            {
                doDelete();
            }
#else
            doDelete();
#endif
            loggerInstance->Info("Berry VM destroyed");
        }
    });
