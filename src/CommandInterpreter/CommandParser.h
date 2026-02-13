#pragma once

#define COMMAND_DELIMITER " "

class CommandParser
{
public:
    static String GetCommandName(String command)
    {
        return CommandParser::GetCommandParameter(command, 0);
    }

    static String GetCommandParameter(String command, uint8_t parameterNo)
    {
        int str_len = command.length() + 1;
        char buf[str_len];
        command.toCharArray(buf, str_len);
        char *p = buf;
        char *str = NULL;
        int currentSegment = 0;
        while (currentSegment++ <= parameterNo && (str = strtok_r(p, COMMAND_DELIMITER, &p)) != NULL)
        { // delimiter is the semicolon
          // Serial.print("Segment ");
          // Serial.print(currentSegment);
          // Serial.print(" is ");
          // Serial.println(str);
        }
        return str ? String(str) : "";
    }
};