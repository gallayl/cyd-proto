#include "SerialRead.h"
#include <string>
#include "../../ActionRegistry/ActionRegistry.h"
#include "../../utils/StringUtil.h"

#include "esp_log.h"
#include "driver/uart.h"

static constexpr size_t MAX_SERIAL_INPUT = 256;
static constexpr uart_port_t UART_NUM = UART_NUM_0;

Feature *serialReadFeature = new Feature(
    "SerialRead",
    []() -> FeatureState
    {
        uart_config_t uart_config = {
            .baud_rate = 115200,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
            .source_clk = UART_SCLK_DEFAULT,
        };
        uart_param_config(UART_NUM, &uart_config);
        uart_driver_install(UART_NUM, MAX_SERIAL_INPUT * 2, 0, 0, NULL, 0);
        return FeatureState::RUNNING;
    },
    []()
    {
        char buf[MAX_SERIAL_INPUT];
        int len = uart_read_bytes(UART_NUM, buf, MAX_SERIAL_INPUT - 1, pdMS_TO_TICKS(10));
        if (len > 0)
        {
            buf[len] = '\0';
            std::string command(buf);
            StringUtil::replaceAll(command, "\r", "");
            StringUtil::replaceAll(command, "\n", "");
            if (!command.empty())
            {
                std::string response = actionRegistryInstance->execute(command, Transport::CLI);
                ESP_LOGI("SerialRead", "%s", response.c_str());
            }
        }
    });
