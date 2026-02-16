#include "i2c.h"
#include "../../ActionRegistry/ActionRegistry.h"

// CYD: GPIO 21 is display backlight; use GPIO 27 (SDA) and 22 (SCL) to avoid conflict
#define I2C_SDA_PIN 27
#define I2C_SCL_PIN 22

FeatureAction i2cAction = {.name = "i2c",
                           .handler =
                               [](const String &command)
                           {
                               String sub = CommandParser::getCommandParameter(command, 1);
                               if (sub == "scan")
                               {
                                   return scanDevices();
                               }
                               else if (sub == "read")
                               {
                                   uint16_t address = CommandParser::getCommandParameter(command, 2).toInt();
                                   uint16_t size = CommandParser::getCommandParameter(command, 3).toInt();
                                   return readDevice(address, size);
                               }
                               else if (sub == "write")
                               {
                                   uint16_t address =
                                       strtol(CommandParser::getCommandParameter(command, 2).c_str(), 0, 16);
                                   String prefix = "i2c write " + CommandParser::getCommandParameter(command, 2) + " ";
                                   String data = command.substring(prefix.length());
                                   writeDevice(address, data);
                                   return String("Written.");
                               }

                               String fallback = "The available I2C Commands are: scan, read, write";
                               return fallback;
                           },
                           .transports = {.cli = true, .rest = true, .ws = true, .scripting = true}};

Feature *i2cFeature = new Feature(
    "i2c",
    []()
    {
        Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

        actionRegistryInstance->registerAction(&i2cAction);

        return FeatureState::RUNNING;
    },
    []() {}, []() { Wire.end(); });
