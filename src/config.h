#define HTTP_PORT 80

#define WEBSOCKETS_URL "/ws"

#define STA_SSID "sticky"
#define STA_PASSPHRASE "sticky1234"

#define STA_IP "192.168.0.1"
#define STA_GATEWAY "192.168.0.1"
#define STA_NETMASK "255.255.255.0"

#define JSON_BUFFER_SIZE 2048

// --- Network ---

/**
 * Enable the WiFi subsystem
 */
#define ENABLE_WIFI true

/**
 * Enable the HTTP web server and WebSocket server
 */
#define ENABLE_WEBSERVER false

// --- Hardware ---

/**
 * Enable the TFT screen initialization
 */
#define ENABLE_SCREEN true

/**
 * Enable the I2C bus feature
 */
#define ENABLE_I2C false

// --- Features ---

/**
 * Enables the LittleFS File System feature
 */
#define ENABLE_LITTLEFS true

/**
 * Enable to read input from a serial console
 */
#define ENABLE_SERIAL_READ true

/**
 * Enable the OTA firmware upgrade feature
 */
#define ENABLE_OTA false

/**
 * Enable the UI/display feature (TFT + touch)
 */
#define ENABLE_UI true

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
