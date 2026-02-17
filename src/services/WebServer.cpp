#include "../config.h"

#if ENABLE_WEBSERVER

#include "WebServer.h"
#include "../mime.h"
#include "../utils/System.h"
#include <string>

#include <esp_http_server.h>
#include <esp_log.h>
#include <sys/stat.h>
#include <cstdio>
#include <unistd.h>
#include "../fs/VirtualFS.h"
#include "../fs/LittleFsInit.h"
#include "../utils/StringUtil.h"
#include "../utils/MultipartParser.h"
#include "../utils/CJsonHelper.h"
#include "../api/list.h"
#include "WebSocketServer.h"

static const char *TAG = "WebServer";
static httpd_handle_t s_server = nullptr;

httpd_handle_t getHttpServer()
{
    return s_server;
}

void stopWebServer()
{
    if (s_server)
    {
        httpd_stop(s_server);
        s_server = nullptr;
    }
}

// --- MIME type helper ---

static const char *getMimeType(const std::string &path)
{
    if (StringUtil::endsWith(path, ".html") || StringUtil::endsWith(path, ".htm"))
        return MIME_HTML;
    if (StringUtil::endsWith(path, ".js"))
        return "application/javascript";
    if (StringUtil::endsWith(path, ".css"))
        return "text/css";
    if (StringUtil::endsWith(path, ".json"))
        return MIME_JSON;
    if (StringUtil::endsWith(path, ".png"))
        return "image/png";
    if (StringUtil::endsWith(path, ".jpg") || StringUtil::endsWith(path, ".jpeg"))
        return MIME_JPEG;
    if (StringUtil::endsWith(path, ".gif"))
        return "image/gif";
    if (StringUtil::endsWith(path, ".ico"))
        return "image/x-icon";
    if (StringUtil::endsWith(path, ".svg"))
        return "image/svg+xml";
    if (StringUtil::endsWith(path, ".woff"))
        return "font/woff";
    if (StringUtil::endsWith(path, ".woff2"))
        return "font/woff2";
    if (StringUtil::endsWith(path, ".be"))
        return MIME_PLAIN_TEXT;
    return "application/octet-stream";
}

// --- /heap endpoint ---

static esp_err_t heapGetHandler(httpd_req_t *req)
{
    std::string heap = std::to_string(getFreeHeap());
    httpd_resp_set_type(req, MIME_PLAIN_TEXT);
    return httpd_resp_send(req, heap.c_str(), HTTPD_RESP_USE_STRLEN);
}

// --- /listFiles endpoint ---

static esp_err_t listFilesHandler(httpd_req_t *req)
{
    cJSON *response = nullptr;

    char queryBuf[256] = {};
    if (httpd_req_get_url_query_str(req, queryBuf, sizeof(queryBuf)) == ESP_OK)
    {
        char pathVal[128] = {};
        if (httpd_query_key_value(queryBuf, "path", pathVal, sizeof(pathVal)) == ESP_OK)
        {
            std::string path(pathVal);
            ResolvedPath resolved = resolveVirtualPath(path);
            if (resolved.valid)
            {
                response = getFileList(resolved.realPath);
            }
            else
            {
                response = getFileList(resolveToLittleFsPath(path));
            }
        }
        else
        {
            response = getFileList();
        }
    }
    else
    {
        response = getFileList();
    }

    std::string responseStr = cJsonToString(response);
    cJSON_Delete(response);
    httpd_resp_set_type(req, MIME_JSON);
    return httpd_resp_send(req, responseStr.c_str(), HTTPD_RESP_USE_STRLEN);
}

// --- /uploadFiles endpoint ---

static std::string sanitizePath(const std::string &raw)
{
    std::string path = raw;
    StringUtil::replaceAll(path, "\\", "/");
    path = StringUtil::trim(path);

    while (path.find("..") != std::string::npos)
        StringUtil::replaceAll(path, "..", "");

    while (path.find("//") != std::string::npos)
        StringUtil::replaceAll(path, "//", "/");

    if (!StringUtil::startsWith(path, "/"))
        path = std::string("/") + path;

    if (path.length() > 1 && StringUtil::endsWith(path, "/"))
        path.erase(path.length() - 1);

    if (path.length() <= 1)
        return "";

    return path;
}

static esp_err_t uploadFilesHandler(httpd_req_t *req)
{
    std::string targetPath;
    std::string basePath;
    std::string errorMsg;

    esp_err_t ret = parseMultipartRequest(
        req,
        [&](const std::string &name, const std::string &value)
        {
            if (name == "path")
                targetPath = value;
            else if (name == "basePath")
                basePath = value;
        },
        [&](const std::string & /*fieldName*/, const std::string &fileName, const uint8_t *data, size_t len,
            bool isFirst, bool isFinal) -> bool
        {
            if (isFirst)
            {
                if (targetPath.empty() && !basePath.empty())
                {
                    if (!StringUtil::endsWith(basePath, "/"))
                        basePath += "/";
                    targetPath = basePath + fileName;
                }
                if (targetPath.empty())
                    targetPath = fileName;

                std::string safePath = sanitizePath(targetPath);
                if (safePath.empty())
                {
                    errorMsg = "{\"error\":\"Invalid file path\"}";
                    return false;
                }

                ResolvedPath resolved = resolveVirtualPath(safePath);
                if (!resolved.valid)
                {
                    errorMsg = "{\"error\":\"Path must start with /flash or /sd\"}";
                    return false;
                }

                vfsMkdirs(resolved.realPath);

                // Store resolved path for subsequent chunks
                targetPath = resolved.realPath;
            }

            const char *mode = isFirst ? "w" : "a";
            FILE *f = fopen(targetPath.c_str(), mode);
            if (!f)
            {
                errorMsg = "{\"error\":\"Failed to open file for writing\"}";
                return false;
            }
            if (data && len > 0)
                fwrite(data, 1, len, f);
            fclose(f);

            if (isFinal)
            {
                loggerInstance->Info(std::string("Upload finished: ") + targetPath);
            }
            return true;
        });

    if (!errorMsg.empty())
    {
        httpd_resp_set_type(req, MIME_JSON);
        httpd_resp_set_status(req, "400 Bad Request");
        return httpd_resp_send(req, errorMsg.c_str(), HTTPD_RESP_USE_STRLEN);
    }

    if (ret != ESP_OK)
    {
        httpd_resp_set_type(req, MIME_JSON);
        httpd_resp_set_status(req, "500 Internal Server Error");
        return httpd_resp_send(req, "{\"error\":\"Upload failed\"}", HTTPD_RESP_USE_STRLEN);
    }

    httpd_resp_set_type(req, MIME_JSON);
    return httpd_resp_send(req, "{\"status\":\"ok\"}", HTTPD_RESP_USE_STRLEN);
}

// --- Static file serving via 404 error handler ---

static esp_err_t staticFileHandler(httpd_req_t *req, httpd_err_code_t err)
{
    std::string uri(req->uri);

    // Strip query string
    auto qpos = uri.find('?');
    if (qpos != std::string::npos)
        uri = uri.substr(0, qpos);

    if (uri == "/" || uri.empty())
        uri = "/index.html";

    std::string filepath = std::string(LITTLEFS_MOUNT_POINT) + uri;

    struct stat st;
    if (stat(filepath.c_str(), &st) != 0)
    {
        httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Not found");
        return ESP_OK;
    }

    httpd_resp_set_type(req, getMimeType(uri));
    httpd_resp_set_hdr(req, "Cache-Control", "max-age=600");

    FILE *f = fopen(filepath.c_str(), "r");
    if (!f)
    {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read file");
        return ESP_OK;
    }

    char buf[1024];
    size_t readBytes;
    do
    {
        readBytes = fread(buf, 1, sizeof(buf), f);
        if (readBytes > 0)
        {
            if (httpd_resp_send_chunk(req, buf, readBytes) != ESP_OK)
            {
                fclose(f);
                httpd_resp_send_chunk(req, nullptr, 0);
                return ESP_FAIL;
            }
        }
    } while (readBytes > 0);

    fclose(f);
    httpd_resp_send_chunk(req, nullptr, 0);
    return ESP_OK;
}

// --- Session close callback (delegates to WebSocket cleanup) ---

static void onHttpSessionClose(httpd_handle_t hd, int sockfd)
{
    wsOnSessionClose(sockfd);
    close(sockfd);
}

// --- Server initialization ---

void initWebServer()
{
    loggerInstance->Info("Starting WEB server");

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 24;
    config.close_fn = onHttpSessionClose;
    config.lru_purge_enable = true;
    config.stack_size = 8192;

    if (httpd_start(&s_server, &config) != ESP_OK)
    {
        loggerInstance->Error("Failed to start HTTP server");
        return;
    }

    // /heap
    const httpd_uri_t heapUri = {.uri = "/heap", .method = HTTP_GET, .handler = heapGetHandler, .user_ctx = nullptr};
    httpd_register_uri_handler(s_server, &heapUri);

    // /listFiles
    const httpd_uri_t listUri = {
        .uri = "/listFiles", .method = HTTP_GET, .handler = listFilesHandler, .user_ctx = nullptr};
    httpd_register_uri_handler(s_server, &listUri);

    // /uploadFiles
    const httpd_uri_t uploadUri = {
        .uri = "/uploadFiles", .method = HTTP_POST, .handler = uploadFilesHandler, .user_ctx = nullptr};
    httpd_register_uri_handler(s_server, &uploadUri);

    // 404 handler → static file serving from LittleFS
    httpd_register_err_handler(s_server, HTTPD_404_NOT_FOUND, staticFileHandler);

    loggerInstance->Info("Server setup done.");
}

#endif // ENABLE_WEBSERVER
