# CYD Proto — ESP32 PlatformIO convenience targets
# Environment name from platformio.ini
ENV = cyd

.PHONY: build upload uploadfs monitor upload-monitor clean check format format-check test berry-clean compiledb

## Build firmware
build:
	pio run -e $(ENV)

## Upload firmware to device
upload:
	pio run -e $(ENV) -t upload

## Upload LittleFS filesystem to device
uploadfs:
	pio run -e $(ENV) -t uploadfs

## Open serial monitor
monitor:
	pio device monitor

## Upload firmware then open serial monitor
upload-monitor:
	pio run -e $(ENV) -t upload && pio device monitor

## Clean build artifacts
clean:
	pio run -e $(ENV) -t clean

## Run clang-tidy static analysis
check:
	pio check -e $(ENV)

## Auto-format source files with clang-format
format:
	@find src -name '*.cpp' -o -name '*.h' -o -name '*.hpp' | xargs clang-format -i

## Check formatting (dry-run, fails on diff)
format-check:
	@find src -name '*.cpp' -o -name '*.h' -o -name '*.hpp' | xargs clang-format --dry-run -Werror

## Run native unit tests
test:
	pio test -e native

## Force Berry constant table regeneration on next build
berry-clean:
	@echo Removing Berry generate directory to trigger regeneration...
	pio run -e $(ENV) -t clean
	@echo Berry tables will regenerate on next build.

## Rebuild IntelliSense index (compile_commands.json)
compiledb:
	pio run -e $(ENV) -t compiledb

## Show available targets
help:
	@echo Available targets:
	@echo   build          - Build firmware
	@echo   upload         - Upload firmware to device
	@echo   uploadfs       - Upload LittleFS filesystem
	@echo   monitor        - Open serial monitor
	@echo   upload-monitor - Upload firmware + open serial monitor
	@echo   clean          - Clean build artifacts
	@echo   check          - Run clang-tidy static analysis
	@echo   format         - Auto-format source files with clang-format
	@echo   format-check   - Check formatting (fails on diff)
	@echo   test           - Run native unit tests
	@echo   berry-clean    - Force Berry table regeneration on next build
	@echo   compiledb      - Rebuild IntelliSense index
	@echo   help           - Show this help
