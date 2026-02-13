#pragma once

#include <Arduino.h>
#include <vector>

#define COMMAND_DELIMITER " "

class CommandParser
{
public:
    static String GetCommandName(const String &command)
    {
        return CommandParser::GetCommandParameter(command, 0);
    }

    static String GetCommandParameter(const String &command, uint8_t parameterNo)
    {
        size_t str_len = command.length() + 1;
        std::vector<char> buf(str_len);
        command.toCharArray(buf.data(), str_len);
        char *p = buf.data();
        char *str = NULL;
        int currentSegment = 0;
        while (currentSegment++ <= parameterNo && (str = strtok_r(p, COMMAND_DELIMITER, &p)) != NULL)
        {
        }
        return str ? String(str) : "";
    }
};