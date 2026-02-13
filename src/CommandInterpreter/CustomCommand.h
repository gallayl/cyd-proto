#pragma once

#include <Arduino.h>

// forward declare parser used in some callbacks
class CommandParser;

typedef String (*CommandCallbackFunction)(const String &command);

class CustomCommand
{
public:
    CustomCommand(const String &name = "commandName", CommandCallbackFunction callback = [](const String &command)
                                                      { return String("Unknown command."); }) : _commandName(name), _onExecute(callback) {};

    String Execute(const String &command)
    {
        return this->_onExecute(command);
    }

    const String &GetCommandName() const
    {
        return this->_commandName;
    }

protected:
    String _commandName;
    CommandCallbackFunction _onExecute;
};
