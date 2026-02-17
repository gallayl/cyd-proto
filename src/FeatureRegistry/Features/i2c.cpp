#include "i2c.h"
#include "../../ActionRegistry/ActionRegistry.h"
#include "../../ActionRegistry/FeatureAction.h"
#include "../../CommandInterpreter/CommandParser.h"

// CYD: GPIO 21 is display backlight; use GPIO 27 (SDA) and 22 (SCL) to avoid conflict
#define I2C_SDA_PIN 27
#define I2C_SCL_PIN 22

i2c_master_bus_handle_t i2cBusHandle = nullptr;

static FeatureAction i2cAction = {
    .name = "i2c",
    .handler =
        [](const std::string &command)
    {
        std::string sub = CommandParser::getCommandParameter(command, 1);
        if (sub == "scan")
        {
            return scanDevices();
        }
        else if (sub == "read")
        {
            uint16_t address = atoi(CommandParser::getCommandParameter(command, 2).c_str());
            uint16_t size = atoi(CommandParser::getCommandParameter(command, 3).c_str());
            return readDevice(address, size);
        }
        else if (sub == "write")
        {
            uint16_t address = strtol(CommandParser::getCommandParameter(command, 2).c_str(), 0, 16);
            std::string prefix = std::string("i2c write ") + CommandParser::getCommandParameter(command, 2) + " ";
            std::string data = command.substr(prefix.length());
            writeDevice(address, data);
            return std::string("Written.");
        }

        std::string fallback = "The available I2C Commands are: scan, read, write";
        return fallback;
    },
    .transports = {.cli = true, .rest = true, .ws = true, .scripting = true}};

Feature *i2cFeature = new Feature(
    "i2c",
    []()
    {
        i2c_master_bus_config_t bus_cfg = {};
        bus_cfg.i2c_port = I2C_NUM_0;
        bus_cfg.sda_io_num = (gpio_num_t)I2C_SDA_PIN;
        bus_cfg.scl_io_num = (gpio_num_t)I2C_SCL_PIN;
        bus_cfg.clk_source = I2C_CLK_SRC_DEFAULT;
        bus_cfg.glitch_ignore_cnt = 7;
        bus_cfg.flags.enable_internal_pullup = true;

        esp_err_t err = i2c_new_master_bus(&bus_cfg, &i2cBusHandle);
        if (err != ESP_OK)
        {
            return FeatureState::ERROR;
        }

        actionRegistryInstance->registerAction(&i2cAction);

        return FeatureState::RUNNING;
    },
    []() {},
    []()
    {
        if (i2cBusHandle != nullptr)
        {
            i2c_del_master_bus(i2cBusHandle);
            i2cBusHandle = nullptr;
        }
    });
