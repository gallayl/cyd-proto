#pragma once

#include <esp_http_server.h>
#include <string>
#include <functional>
#include <cstring>
#include <cctype>

using MultipartFieldCb = std::function<void(const std::string &name, const std::string &value)>;
using MultipartFileDataCb = std::function<bool(const std::string &fieldName, const std::string &fileName,
                                               const uint8_t *data, size_t len, bool isFirst, bool isFinal)>;

inline std::string mpExtractBoundary(const char *contentType)
{
    if (!contentType)
        return "";
    const char *b = strstr(contentType, "boundary=");
    if (!b)
        return "";
    b += 9;
    if (*b == '"')
    {
        b++;
        const char *end = strchr(b, '"');
        return end ? std::string(b, end - b) : "";
    }
    const char *end = b;
    while (*end && *end != ';' && *end != ' ' && *end != '\r' && *end != '\n')
        end++;
    return std::string(b, end - b);
}

inline void mpParsePartHeaders(const std::string &headers, std::string &name, std::string &filename)
{
    name.clear();
    filename.clear();

    std::string lower = headers;
    for (auto &c : lower)
        c = tolower(static_cast<unsigned char>(c));

    size_t cdPos = lower.find("content-disposition:");
    if (cdPos == std::string::npos)
        return;

    size_t lineEnd = headers.find("\r\n", cdPos);
    std::string cdLine = headers.substr(cdPos, lineEnd == std::string::npos ? std::string::npos : lineEnd - cdPos);

    size_t namePos = cdLine.find("name=\"");
    if (namePos != std::string::npos)
    {
        namePos += 6;
        size_t nameEnd = cdLine.find('"', namePos);
        if (nameEnd != std::string::npos)
            name = cdLine.substr(namePos, nameEnd - namePos);
    }

    size_t fnPos = cdLine.find("filename=\"");
    if (fnPos != std::string::npos)
    {
        fnPos += 10;
        size_t fnEnd = cdLine.find('"', fnPos);
        if (fnEnd != std::string::npos)
            filename = cdLine.substr(fnPos, fnEnd - fnPos);
    }
}

/**
 * Streaming multipart/form-data parser for esp_http_server.
 *
 * Reads the request body in chunks and dispatches callbacks:
 *  - onField(name, value) for non-file form fields
 *  - onFileData(fieldName, fileName, data, len, isFirst, isFinal) for file data chunks
 *    Return false from onFileData to abort parsing.
 */
inline esp_err_t parseMultipartRequest(httpd_req_t *req, MultipartFieldCb onField, MultipartFileDataCb onFileData)
{
    char ctBuf[256] = {};
    if (httpd_req_get_hdr_value_str(req, "Content-Type", ctBuf, sizeof(ctBuf)) != ESP_OK)
        return ESP_ERR_INVALID_ARG;

    std::string boundary = mpExtractBoundary(ctBuf);
    if (boundary.empty())
        return ESP_ERR_INVALID_ARG;

    std::string boundaryMarker = "\r\n--" + boundary;
    std::string firstBoundary = "--" + boundary + "\r\n";

    enum State
    {
        PREAMBLE,
        HEADER,
        BODY,
        AFTER_BOUNDARY
    };
    State state = PREAMBLE;

    std::string currentName, currentFileName;
    bool isFile = false;
    bool isFirstChunk = true;
    std::string fieldValue;
    std::string buffer;

    const size_t READ_SZ = 2048;
    char readBuf[READ_SZ];
    int remaining = req->content_len;

    while (true)
    {
        // --- Process buffer as far as possible ---
        bool progress = true;
        while (progress)
        {
            progress = false;

            if (state == PREAMBLE)
            {
                size_t pos = buffer.find(firstBoundary);
                if (pos != std::string::npos)
                {
                    buffer.erase(0, pos + firstBoundary.length());
                    state = HEADER;
                    progress = true;
                }
            }

            if (state == HEADER)
            {
                size_t pos = buffer.find("\r\n\r\n");
                if (pos != std::string::npos)
                {
                    std::string headers = buffer.substr(0, pos);
                    buffer.erase(0, pos + 4);

                    mpParsePartHeaders(headers, currentName, currentFileName);
                    isFile = !currentFileName.empty();
                    isFirstChunk = true;
                    fieldValue.clear();
                    state = BODY;
                    progress = true;
                }
            }

            if (state == BODY)
            {
                size_t pos = buffer.find(boundaryMarker);

                if (pos != std::string::npos)
                {
                    if (isFile && onFileData)
                    {
                        bool cont = onFileData(currentName, currentFileName, (const uint8_t *)buffer.data(), pos,
                                               isFirstChunk, true);
                        if (!cont)
                            return ESP_ERR_INVALID_STATE;
                    }
                    else if (!isFile)
                    {
                        fieldValue.append(buffer.data(), pos);
                        if (onField)
                            onField(currentName, fieldValue);
                    }

                    buffer.erase(0, pos + boundaryMarker.length());
                    state = AFTER_BOUNDARY;
                    progress = true;
                }
                else
                {
                    // Flush safe data, keeping tail to detect split boundary
                    if (buffer.size() > boundaryMarker.size())
                    {
                        size_t safeLen = buffer.size() - boundaryMarker.size();
                        if (isFile && onFileData)
                        {
                            bool cont = onFileData(currentName, currentFileName, (const uint8_t *)buffer.data(),
                                                   safeLen, isFirstChunk, false);
                            isFirstChunk = false;
                            if (!cont)
                                return ESP_ERR_INVALID_STATE;
                        }
                        else if (!isFile)
                        {
                            fieldValue.append(buffer.data(), safeLen);
                        }
                        buffer.erase(0, safeLen);
                    }
                }
            }

            if (state == AFTER_BOUNDARY)
            {
                if (buffer.size() >= 2)
                {
                    if (buffer[0] == '-' && buffer[1] == '-')
                    {
                        return ESP_OK;
                    }
                    buffer.erase(0, 2); // skip \r\n
                    state = HEADER;
                    progress = true;
                }
            }
        }

        // --- Read more data ---
        if (remaining <= 0)
            break;

        int toRead = remaining < (int)READ_SZ ? remaining : (int)READ_SZ;
        int recvd = httpd_req_recv(req, readBuf, toRead);
        if (recvd <= 0)
        {
            if (recvd == HTTPD_SOCK_ERR_TIMEOUT)
                continue;
            break;
        }
        remaining -= recvd;
        buffer.append(readBuf, recvd);
    }

    // Handle remaining data at end of stream
    if (state == BODY && !buffer.empty())
    {
        if (isFile && onFileData)
        {
            onFileData(currentName, currentFileName, (const uint8_t *)buffer.data(), buffer.size(), isFirstChunk, true);
        }
        else if (!isFile && onField)
        {
            fieldValue.append(buffer);
            onField(currentName, fieldValue);
        }
    }

    return ESP_OK;
}
