#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#define COMMAND_DELIMITER " "

class CommandParser
{
public:
    static std::string getCommandName(const char *command)
    {
        return getCommandParameter(command, 0);
    }

    static std::string getCommandParameter(const char *command, uint8_t parameterNo)
    {
        if (command == nullptr)
        {
            return "";
        }
        size_t len = std::strlen(command) + 1;
        std::vector<char> buf(len);
        std::memcpy(buf.data(), command, len);
        char *p = buf.data();
        char *str = nullptr;
        int currentSegment = 0;
        while (currentSegment++ <= parameterNo && (str = strtok_r(p, COMMAND_DELIMITER, &p)) != nullptr)
        {
        }
        return str != nullptr ? std::string(str) : "";
    }

    static std::string getCommandName(const std::string &command)
    {
        return getCommandParameter(command.c_str(), 0);
    }

    static std::string getCommandParameter(const std::string &command, uint8_t parameterNo)
    {
        return getCommandParameter(command.c_str(), parameterNo);
    }
};