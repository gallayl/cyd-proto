# CYD Proto

ESP32 firmware for a CYD (Cheap Yellow Display) device featuring a touchscreen UI, web interface, and Berry scripting support.

## Features

- Touchscreen UI with window manager (LovyanGFX)
- WiFi connectivity with async web server & WebSocket support
- Berry scripting language for runtime extensibility
- LittleFS & SD card storage
- OTA firmware updates
- REST API
- Serial command interpreter

## Prerequisites

- [PlatformIO CLI](https://docs.platformio.org/en/latest/core/installation/index.html) or [PlatformIO IDE extension](https://platformio.org/install/ide?install=vscode)
- Python 3 (for Berry code generation)
- GNU Make _(optional, for Makefile targets)_
  - Windows: `choco install make`, `winget install GnuWin32.Make`, or `scoop install make`

## Getting Started

1. Clone the repository
2. Build and upload:
   ```sh
   make upload-monitor
   ```
   Or using PlatformIO directly:
   ```sh
   pio run -e cyd -t upload && pio device monitor
   ```

## Build Commands

| Command               | Description                                  |
| --------------------- | -------------------------------------------- |
| `make build`          | Compile firmware                             |
| `make upload`         | Flash firmware to device                     |
| `make uploadfs`       | Upload LittleFS filesystem to device         |
| `make monitor`        | Open serial monitor                          |
| `make upload-monitor` | Flash firmware + open serial monitor         |
| `make clean`          | Clean build artifacts                        |
| `make check`          | Run clang-tidy static analysis               |
| `make berry-clean`    | Force Berry table regeneration on next build |
| `make help`           | List all available targets                   |

## Project Structure

```
src/
├── main.cpp               # Arduino entry point
├── config.h               # Feature flags
├── FeatureRegistry/       # Feature-based architecture
│   └── Features/          # UI, Berry, Time, Logging, OTA, …
├── hw/                    # Hardware abstraction (WiFi, Screen)
├── services/              # WebServer, WebSocketServer
├── api/                   # REST API endpoints
├── berry/                 # Berry language integration
├── ActionRegistry/        # Action system
└── CommandInterpreter/    # Serial command handling
data/                      # LittleFS filesystem (uploaded to device)
```

## Configuration

Feature flags are toggled in `src/config.h`:

- `ENABLE_WIFI`, `ENABLE_WEBSERVER`, `ENABLE_SCREEN`
- `ENABLE_I2C`, `ENABLE_LITTLEFS`, `ENABLE_SD_CARD`
- `ENABLE_SERIAL_READ`, `ENABLE_OTA`, `ENABLE_UI`, `ENABLE_BERRY`
