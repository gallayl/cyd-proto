#pragma once

typedef String (*CommandCallbackFunction)(String command);

class CustomCommand
{
public:
    CustomCommand(String name = "commandName", CommandCallbackFunction callback = [](String command)
                                               { return String("Unknown command."); }) : _commandName(name), _onExecute(callback) {};

    String Execute(String command)
    {
        return this->_onExecute(command);
    }

    String GetCommandName()
    {
        return this->_commandName;
    }

protected:
    String _commandName;
    CommandCallbackFunction _onExecute;
};
