#pragma once
#include "./CustomCommand.h"
#include "./CommandParser.h"

#define COMMANDS_SIZE 128

class CommandInterpreter
{
private:
    uint8_t _registeredCommandsCount = 0;
public:

    CommandInterpreter()
    {
            // this->RegisterCommand(*infoAction);
    }

    void RegisterCommand(CustomCommand newCommand)
    {
        this->RegisteredCommands[this->_registeredCommandsCount] = newCommand;
        this->_registeredCommandsCount++;
    }

    String getAvailableCommands()
    {
        String commands = "";

        uint16_t commandId;
        for (commandId = 0; commandId < this->_registeredCommandsCount; commandId++)
        {
            commands += this->RegisteredCommands[commandId].GetCommandName() + ", ";
        };
        return commands;
    }

    String ExecuteCommand(String command)
    {
        for (uint8_t i = 0; i < COMMANDS_SIZE; i++)
        {
            String commandName = this->RegisteredCommands[i].GetCommandName();
            if (command.equals(commandName) || command.startsWith(commandName + " "))
            {
                String result = this->RegisteredCommands[i].Execute(command);
                return result;
            }
        }
        return String("{\"message\": \"Unknown command: " + CommandParser::GetCommandName(command) + ".\", \"availableCommands\": \"" + this->getAvailableCommands() + "\"}");
    }

    CustomCommand RegisteredCommands[COMMANDS_SIZE];
};


CommandInterpreter *CommandInterpreterInstance = new CommandInterpreter();