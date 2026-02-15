#define HTTP_PORT 80

#define WEBSOCKETS_URL "/ws"

#define STA_SSID "sticky"
#define STA_PASSPHRASE "sticky1234"

#define JSON_BUFFER_SIZE 2048

// --- Network ---

/**
 * Enable the WiFi subsystem
 */
#define ENABLE_WIFI true

/**
 * Enable the HTTP web server and WebSocket server
 */
#define ENABLE_WEBSERVER true

// --- Hardware ---

/**
 * Enable the TFT screen initialization
 */
#define ENABLE_SCREEN true

/**
 * Enable the I2C bus feature
 */
#define ENABLE_I2C true

// --- Features ---

/**
 * Enables the LittleFS File System feature
 */
#define ENABLE_LITTLEFS true

/**
 * Enable the SD card feature
 */
#define ENABLE_SD_CARD true

/**
 * Enable to read input from a serial console
 */
#define ENABLE_SERIAL_READ true

/**
 * Enable the OTA firmware upgrade feature
 */
#define ENABLE_OTA true

/**
 * Enable the UI/display feature (TFT + touch)
 */
#define ENABLE_UI true

/**
 * Enable the Berry scripting language VM
 */
#define ENABLE_BERRY true

// --- Compile-time dependency checks ---

#if ENABLE_WEBSERVER && !ENABLE_WIFI
#error "ENABLE_WEBSERVER requires ENABLE_WIFI"
#endif

#if ENABLE_OTA && !ENABLE_WEBSERVER
#error "ENABLE_OTA requires ENABLE_WEBSERVER"
#endif

#if ENABLE_UI && !ENABLE_SCREEN
#error "ENABLE_UI requires ENABLE_SCREEN"
#endif

#if ENABLE_BERRY && !ENABLE_LITTLEFS
#error "ENABLE_BERRY requires ENABLE_LITTLEFS"
#endif
