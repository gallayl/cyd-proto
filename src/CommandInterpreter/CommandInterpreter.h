#pragma once

#include <Arduino.h>
#include "./CustomCommand.h"
#include "./CommandParser.h"

#define COMMANDS_SIZE 24

class CommandInterpreter
{
private:
    uint8_t _registeredCommandsCount = 0;

public:
    CommandInterpreter()
    {
        // this->RegisterCommand(infoAction);
    }

    void RegisterCommand(CustomCommand *newCommand)
    {
        if (this->_registeredCommandsCount >= COMMANDS_SIZE)
        {
            Serial.println(F("Command registry full, cannot register"));
            return;
        }
        this->RegisteredCommands[this->_registeredCommandsCount] = newCommand;
        this->_registeredCommandsCount++;
    }

    String getAvailableCommands()
    {
        String commands = "";

        uint16_t commandId;
        for (commandId = 0; commandId < this->_registeredCommandsCount; commandId++)
        {
            commands += this->RegisteredCommands[commandId]->GetCommandName() + ", ";
        };
        return commands;
    }

    String ExecuteCommand(const String &command)
    {
        for (uint8_t i = 0; i < this->_registeredCommandsCount; i++)
        {
            const String &commandName = this->RegisteredCommands[i]->GetCommandName();
            if (command.equals(commandName) || command.startsWith(commandName + " "))
            {
                String result = this->RegisteredCommands[i]->Execute(command);
                return result;
            }
        }
        return String("{\"message\": \"Unknown command: " + CommandParser::GetCommandName(command) + ".\", \"availableCommands\": \"" + this->getAvailableCommands() + "\"}");
    }

    CustomCommand *RegisteredCommands[COMMANDS_SIZE] = {};
};

extern CommandInterpreter *CommandInterpreterInstance;