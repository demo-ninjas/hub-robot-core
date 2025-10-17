# WifiManager Class - Usage Guide

## Quick Start

```cpp
#include "wifi_manager.h"

// Create WiFi manager with your network credentials
WifiManager wifi("YourSSID", "YourPassword");

void setup() {
    Serial.begin(115200);
    
    // Set up callbacks
    wifi.onConnected([](String ip) {
        Serial.println("Connected! IP: " + ip);
    });
    
    wifi.onDisconnected([]() {
        Serial.println("Disconnected from WiFi");
    });
    
    // Start connection
    wifi.begin();
}

void loop() {
    // Your application code
    if (wifi.isConnected()) {
        // Do network operations
    }
}
```

## Overview

The `WifiManager` class provides a simplified, cross-platform interface for managing WiFi connections on ESP32 and Arduino boards with WiFi capabilities. It handles connection management, automatic reconnection, and provides event-driven callbacks for connection state changes.

**Key Features:**
- Simple connection management with SSID and password
- Asynchronous event handling (ESP32) and synchronous connection (other platforms)
- Automatic reconnection on connection loss
- Connection state callbacks
- Optional logging support
- Signal strength monitoring
- Cross-platform compatibility (ESP32, WiFiNINA boards)

## Platform Support

### ESP32
- Uses WiFi event system for asynchronous connection handling
- Non-blocking `begin()` method
- Event-driven state management
- Recommended for most ESP32 projects

### Arduino with WiFiNINA (e.g., Arduino Nano 33 IoT)
- Synchronous connection with timeout
- Blocking `begin()` method (up to 10 seconds)
- Polling-based connection management

## Constructor

```cpp
WifiManager(String ssid, String pass);
```

### Parameters

- **ssid**: WiFi network name (SSID)
  - Case-sensitive
  - Maximum 32 characters (WiFi standard)
  - Can include spaces and special characters
  
- **pass**: WiFi network password
  - Case-sensitive
  - 8-63 characters for WPA/WPA2
  - Can be empty for open networks (not recommended)

### Example

```cpp
// Standard home network
WifiManager wifi("MyHomeNetwork", "SecurePassword123");

// Network with special characters
WifiManager wifi("WiFi@Home-5G", "P@ssw0rd!2024");

// Open network (no password)
WifiManager wifi("GuestNetwork", "");
```

### Initial State

After construction:
- Status: `WifiStatus::IDLE`
- Connected: `false`
- Auto-reconnect: `true` (enabled by default)
- IP address: Empty string
- No active callbacks

## Core API

### begin() - Start Connection

```cpp
void begin();
```

Initiates WiFi connection to the configured network.

**Behavior:**
- **ESP32**: Returns immediately, connection happens asynchronously
- **Other platforms**: Blocks until connected or timeout (10 seconds)

**ESP32 Connection Sequence:**
1. `begin()` called → Status becomes `CONNECTING`
2. WiFi hardware starts association
3. On successful connection → `CONNECTED` event → Status becomes `CONNECTED`
4. Callback fired with IP address

**Non-ESP32 Connection Sequence:**
1. `begin()` called
2. Waits up to 10 seconds for connection
3. If successful → Status becomes `CONNECTED`, callback fired
4. If failed → Status becomes `ERROR`

**Example:**

```cpp
void setup() {
    Serial.begin(115200);
    
    WifiManager wifi("MyNetwork", "MyPassword");
    wifi.setLogger(Serial);
    
    wifi.begin();
    
    #ifdef ARDUINO_ARCH_ESP32
    // Non-blocking, continue with other setup
    Serial.println("WiFi connection initiated");
    #else
    // Blocking, connection complete or failed
    if (wifi.isConnected()) {
        Serial.println("Connected!");
    } else {
        Serial.println("Connection failed!");
    }
    #endif
}
```

**Notes:**
- Safe to call multiple times
- Returns immediately if already connected
- Does NOT disconnect existing connection

### disconnect() - Disconnect from Network

```cpp
void disconnect();
```

Disconnects from the WiFi network.

**Behavior:**
- Sets status to `DISCONNECTING`
- Calls WiFi library disconnect
- **ESP32**: Triggers `DISCONNECTED` event (callback fired)
- **Other platforms**: Synchronous disconnection

**Example:**

```cpp
void enterSleepMode() {
    wifi.setAutoReconnect(false);  // Prevent reconnection
    wifi.disconnect();
    // Enter deep sleep
}
```

### isConnected() - Check Connection Status

```cpp
bool isConnected();
```

Returns the current connection state.

**Returns:** `true` if connected and has IP address, `false` otherwise

**Example:**

```cpp
void loop() {
    if (wifi.isConnected()) {
        // Perform network operations
        sendData();
    } else {
        // Use offline mode
        cacheData();
    }
}
```

**Important:** This returns the internal state, not live WiFi status. Updated via:
- ESP32: WiFi events
- Other platforms: After `begin()` completes

### status() - Get Connection Status

```cpp
WifiStatus status();
```

Returns detailed connection status.

**Possible Values:**
- `WifiStatus::IDLE` - Not yet attempted connection
- `WifiStatus::CONNECTING` - Connection in progress (ESP32)
- `WifiStatus::CONNECTED` - Connected with IP address
- `WifiStatus::DISCONNECTING` - Disconnection in progress
- `WifiStatus::DISCONNECTED` - Disconnected from network
- `WifiStatus::ERROR` - Connection failed (non-ESP32)

**Example:**

```cpp
void displayStatus() {
    switch (wifi.status()) {
        case WifiStatus::IDLE:
            Serial.println("Idle");
            break;
        case WifiStatus::CONNECTING:
            Serial.println("Connecting...");
            break;
        case WifiStatus::CONNECTED:
            Serial.println("Connected");
            break;
        case WifiStatus::DISCONNECTED:
            Serial.println("Disconnected");
            break;
        case WifiStatus::ERROR:
            Serial.println("Error");
            break;
    }
}
```

## Callback Functions

### onConnected - Connection Established

```cpp
void onConnected(std::function<void(String)> callback);
```

Registers a callback function that fires when WiFi connection is established and IP address is obtained.

**Callback Parameter:**
- `String ip` - The assigned IP address (e.g., "192.168.1.100")

**When it fires:**
- **ESP32**: When `WIFI_STA_GOT_IP` event occurs
- **Other platforms**: After `begin()` successfully connects

**Example:**

```cpp
wifi.onConnected([](String ip) {
    Serial.println("WiFi connected!");
    Serial.println("IP address: " + ip);
    
    // Start network services
    startWebServer();
    connectToMQTT();
});
```

**Notes:**
- Callback is executed synchronously
- Keep callback fast (no long delays or blocking operations)
- Can be called from ISR context on ESP32
- Replaces previous callback if called multiple times

### onDisconnected - Connection Lost

```cpp
void onDisconnected(std::function<void()> callback);
```

Registers a callback function that fires when WiFi connection is lost.

**When it fires:**
- **ESP32**: When `WIFI_STA_DISCONNECTED` event occurs
- **Other platforms**: Not typically used (synchronous connection model)

**Example:**

```cpp
wifi.onDisconnected([]() {
    Serial.println("WiFi disconnected!");
    
    // Stop network services
    stopWebServer();
    disconnectFromMQTT();
    
    // Optional: Indicate status with LED
    digitalWrite(LED_PIN, LOW);
});
```

**Notes:**
- Fires before auto-reconnect attempt (if enabled)
- Keep callback fast
- Can be called from ISR context on ESP32

## Auto-Reconnect

### setAutoReconnect() - Enable/Disable Auto-Reconnect

```cpp
void setAutoReconnect(bool autoReconnect);
```

Enables or disables automatic reconnection on connection loss.

**Parameters:**
- `autoReconnect`: `true` to enable, `false` to disable

**Default:** Enabled (`true`)

**Example:**

```cpp
// Disable auto-reconnect for battery-powered device
wifi.setAutoReconnect(false);

// Enable auto-reconnect for always-on device
wifi.setAutoReconnect(true);
```

**Behavior:**
- **When enabled (ESP32)**: Automatically calls `begin()` when disconnected
- **When disabled**: Remains disconnected after connection loss
- Setting persists across connections

### isAutoReconnect() - Check Auto-Reconnect State

```cpp
bool isAutoReconnect();
```

Returns the current auto-reconnect setting.

**Returns:** `true` if enabled, `false` if disabled

**Example:**

```cpp
if (wifi.isAutoReconnect()) {
    Serial.println("Auto-reconnect is enabled");
}
```

## Network Information

### address() - Get IP Address

```cpp
String address();
```

Returns the current IP address as a string.

**Returns:** IP address (e.g., "192.168.1.100") or empty string if not connected

**Example:**

```cpp
if (wifi.isConnected()) {
    Serial.println("My IP: " + wifi.address());
}
```

### strength() - Get Signal Strength

```cpp
long strength();
```

Returns the current WiFi signal strength (RSSI).

**Returns:** RSSI value in dBm (typically -30 to -90)
- **-30 to -50 dBm**: Excellent signal
- **-50 to -60 dBm**: Good signal
- **-60 to -70 dBm**: Fair signal
- **-70 to -80 dBm**: Weak signal
- **-80 to -90 dBm**: Very weak signal

**Example:**

```cpp
long rssi = wifi.strength();

Serial.print("Signal strength: ");
Serial.print(rssi);
Serial.print(" dBm (");

if (rssi > -50) {
    Serial.println("Excellent)");
} else if (rssi > -60) {
    Serial.println("Good)");
} else if (rssi > -70) {
    Serial.println("Fair)");
} else {
    Serial.println("Weak)");
}
```

**Note:** Can be called even when not connected (returns last known value)

## Logging

### setLogger() - Enable Logging

```cpp
void setLogger(Print& logger);
```

Enables logging of WiFi events to a `Print` object (typically `Serial`).

**Parameters:**
- `logger`: Reference to a `Print` object (e.g., `Serial`, `Serial1`)

**Example:**

```cpp
void setup() {
    Serial.begin(115200);
    
    WifiManager wifi("MyNetwork", "MyPassword");
    wifi.setLogger(Serial);  // Enable logging to Serial
    
    wifi.begin();
}
```

**Log Messages:**
- `WIFI CONNECTING; To network: [SSID]`
- `WIFI CONNECTED; IP: [IP]; RSSI: [RSSI]`
- `WIFI DISCONNECTED; Disconnected from WiFi network`
- `WiFi module not available - cannot connect to WiFi!` (non-ESP32 error)
- `Failed to connect to WiFi network: [SSID]` (non-ESP32 error)

**Notes:**
- Logging is optional
- Pass reference, not pointer: `setLogger(Serial)` not `setLogger(&Serial)`
- Can be called before or after `begin()`

## Common Usage Patterns

### Pattern 1: Basic Connection with Status LED

```cpp
#include "wifi_manager.h"

const int LED_PIN = 2;
WifiManager wifi("MyNetwork", "MyPassword");

void setup() {
    pinMode(LED_PIN, OUTPUT);
    
    wifi.onConnected([](String ip) {
        digitalWrite(LED_PIN, HIGH);  // LED on when connected
    });
    
    wifi.onDisconnected([]() {
        digitalWrite(LED_PIN, LOW);   // LED off when disconnected
    });
    
    wifi.begin();
}

void loop() {
    // Your application code
}
```

### Pattern 2: Web Server with Connection Management

```cpp
#include "wifi_manager.h"
#include <WebServer.h>

WifiManager wifi("MyNetwork", "MyPassword");
WebServer server(80);

void handleRoot() {
    server.send(200, "text/plain", "Hello from ESP32!");
}

void setup() {
    Serial.begin(115200);
    wifi.setLogger(Serial);
    
    wifi.onConnected([](String ip) {
        Serial.println("Starting web server on " + ip);
        
        server.on("/", handleRoot);
        server.begin();
    });
    
    wifi.onDisconnected([]() {
        Serial.println("Web server stopped");
        server.stop();
    });
    
    wifi.begin();
}

void loop() {
    if (wifi.isConnected()) {
        server.handleClient();
    }
}
```

### Pattern 3: MQTT Client with Auto-Reconnect

```cpp
#include "wifi_manager.h"
#include <PubSubClient.h>

WifiManager wifi("MyNetwork", "MyPassword");
WiFiClient espClient;
PubSubClient mqtt(espClient);

void connectMQTT() {
    mqtt.setServer("mqtt.example.com", 1883);
    mqtt.connect("ESP32Client");
}

void setup() {
    wifi.setAutoReconnect(true);
    
    wifi.onConnected([](String ip) {
        connectMQTT();
    });
    
    wifi.onDisconnected([]() {
        mqtt.disconnect();
    });
    
    wifi.begin();
}

void loop() {
    if (wifi.isConnected() && mqtt.connected()) {
        mqtt.loop();
    } else if (wifi.isConnected() && !mqtt.connected()) {
        connectMQTT();
    }
}
```

### Pattern 4: Battery-Powered Device with Manual Control

```cpp
#include "wifi_manager.h"

WifiManager wifi("MyNetwork", "MyPassword");

void setup() {
    wifi.setAutoReconnect(false);  // Disable auto-reconnect to save power
}

void sendDataAndSleep() {
    wifi.begin();
    
    #ifdef ARDUINO_ARCH_ESP32
    // Wait for connection (with timeout)
    unsigned long start = millis();
    while (!wifi.isConnected() && millis() - start < 10000) {
        delay(100);
    }
    #endif
    
    if (wifi.isConnected()) {
        // Send data via HTTP/MQTT
        sendSensorData();
        
        // Disconnect before sleep
        wifi.disconnect();
    }
    
    // Enter deep sleep
    esp_deep_sleep(60 * 1000000);  // Sleep for 60 seconds
}
```

### Pattern 5: Connection Status Display

```cpp
#include "wifi_manager.h"
#include <Wire.h>
#include <Adafruit_SSD1306.h>

WifiManager wifi("MyNetwork", "MyPassword");
Adafruit_SSD1306 display(128, 64, &Wire, -1);

void updateDisplay() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    
    display.println("WiFi Status:");
    
    if (wifi.isConnected()) {
        display.println("Connected");
        display.println("IP: " + wifi.address());
        display.print("RSSI: ");
        display.print(wifi.strength());
        display.println(" dBm");
    } else {
        switch (wifi.status()) {
            case WifiStatus::IDLE:
                display.println("Idle");
                break;
            case WifiStatus::CONNECTING:
                display.println("Connecting...");
                break;
            case WifiStatus::DISCONNECTED:
                display.println("Disconnected");
                break;
            case WifiStatus::ERROR:
                display.println("Error");
                break;
        }
    }
    
    display.display();
}

void setup() {
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    
    wifi.onConnected([](String ip) {
        updateDisplay();
    });
    
    wifi.onDisconnected([]() {
        updateDisplay();
    });
    
    wifi.begin();
}

void loop() {
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate > 1000) {
        lastUpdate = millis();
        updateDisplay();
    }
}
```

### Pattern 6: Multi-Network Fallback

```cpp
#include "wifi_manager.h"

struct NetworkConfig {
    String ssid;
    String password;
};

NetworkConfig networks[] = {
    {"HomeNetwork", "HomePassword"},
    {"OfficeNetwork", "OfficePassword"},
    {"MobileHotspot", "HotspotPassword"}
};

int currentNetwork = 0;
WifiManager* wifi = nullptr;

void tryNextNetwork() {
    if (wifi != nullptr) {
        delete wifi;
    }
    
    wifi = new WifiManager(
        networks[currentNetwork].ssid,
        networks[currentNetwork].password
    );
    
    wifi->setAutoReconnect(false);
    
    wifi->onConnected([](String ip) {
        Serial.println("Connected to network " + String(currentNetwork));
    });
    
    wifi->begin();
    
    // ESP32: Need to wait for connection result
    #ifdef ARDUINO_ARCH_ESP32
    unsigned long start = millis();
    while (!wifi->isConnected() && millis() - start < 10000) {
        delay(100);
    }
    #endif
    
    if (!wifi->isConnected()) {
        currentNetwork = (currentNetwork + 1) % 3;
        tryNextNetwork();
    }
}

void setup() {
    Serial.begin(115200);
    tryNextNetwork();
}
```

## Important Notes

### ⚠️ Platform Differences

**ESP32:**
- `begin()` is non-blocking
- Connection happens asynchronously via events
- Callbacks fired from WiFi event task
- Status updates happen via events

**WiFiNINA (Arduino):**
- `begin()` is blocking (up to 10 seconds)
- Synchronous connection handling
- Disconnect callback rarely used
- Manual status polling required

### ⚠️ Callback Context (ESP32)

Callbacks on ESP32 are called from the WiFi event task:
- **DO NOT** use long delays
- **DO NOT** do heavy processing
- **DO** keep operations fast and simple
- **CONSIDER** setting flags and handling in main loop

**Good:**
```cpp
volatile bool shouldStartServer = false;

wifi.onConnected([](String ip) {
    shouldStartServer = true;
});

void loop() {
    if (shouldStartServer) {
        shouldStartServer = false;
        server.begin();
    }
}
```

**Bad:**
```cpp
wifi.onConnected([](String ip) {
    delay(5000);  // DON'T DO THIS
    heavyProcessing();  // DON'T DO THIS
});
```

### ⚠️ Credential Storage

WiFi credentials are stored as `String` objects:
- Stored in RAM (not Flash)
- Cleared on reset
- Consider security implications for production devices
- For better security, retrieve from secure storage

### ⚠️ Auto-Reconnect Behavior

When auto-reconnect is enabled:
- ESP32: Reconnects on any disconnection (including manual)
- To permanently disconnect: Call `setAutoReconnect(false)` before `disconnect()`

```cpp
// Temporary disconnect (will reconnect)
wifi.disconnect();

// Permanent disconnect
wifi.setAutoReconnect(false);
wifi.disconnect();
```

### ⚠️ Connection Timeout (Non-ESP32)

On non-ESP32 platforms:
- `begin()` blocks for up to 10 seconds
- Cannot be interrupted
- Consider watchdog timer implications
- Plan UI accordingly (show "connecting" message)

### ⚠️ IP Address Caching

The IP address is cached internally:
- Updated on connection
- Cleared on disconnection
- Safe to call `address()` anytime
- Returns last known address if called during connection

### ⚠️ RSSI Availability

`strength()` returns WiFi.RSSI():
- Valid when connected
- May return stale value when disconnected
- Platform-dependent behavior
- Check `isConnected()` before relying on value

## Troubleshooting

### Connection fails on ESP32

**Symptoms:** Never receives `CONNECTED` status

**Possible Causes:**
1. Wrong credentials (SSID/password)
2. Out of range
3. 5GHz network (ESP32 only supports 2.4GHz)
4. MAC filtering on router
5. Network at capacity

**Solutions:**
1. Verify credentials (case-sensitive)
2. Move closer to router
3. Use 2.4GHz network
4. Add ESP32 MAC to router whitelist
5. Check router client limit

### Connection fails on Arduino

**Symptoms:** `begin()` returns with `ERROR` status

**Possible Causes:**
1. WiFi module not detected (`WL_NO_MODULE`)
2. Wrong credentials
3. Timeout (10 seconds)

**Solutions:**
1. Check WiFi module installation and firmware
2. Verify credentials
3. Improve signal strength
4. Check router logs

### Callbacks not firing

**Symptoms:** Connection works but callbacks never called

**Possible Causes:**
1. Callback registered after connection
2. Callback set to `nullptr`
3. Event handler not registered (ESP32)

**Solutions:**
1. Register callbacks before `begin()`
2. Check callback registration
3. Verify event handler (ESP32)

### Auto-reconnect not working

**Symptoms:** Stays disconnected after connection loss

**Possible Causes:**
1. Auto-reconnect disabled
2. Non-ESP32 platform (limited support)
3. WiFi hardware issue

**Solutions:**
1. Call `setAutoReconnect(true)`
2. Implement manual reconnection logic
3. Power cycle WiFi module

### Memory issues / crashes

**Symptoms:** Crashes or memory allocation failures

**Possible Causes:**
1. Too many `WifiManager` instances
2. Callback lambda captures large objects
3. String operations in callbacks
4. Heap fragmentation

**Solutions:**
1. Use single `WifiManager` instance
2. Minimize callback captures
3. Use flag-setting in callbacks, process in loop
4. Avoid String concatenation in callbacks

### Signal strength always shows same value

**Symptoms:** `strength()` returns constant value

**Possible Causes:**
1. Not connected (returns last known value)
2. Platform doesn't update RSSI
3. Hardware limitation

**Solutions:**
1. Check `isConnected()` first
2. Test on different platform
3. Accept platform limitations

## Best Practices

✅ **DO:**
- Register callbacks before calling `begin()`
- Use `setLogger()` during development
- Keep callbacks fast and simple
- Check `isConnected()` before network operations
- Use single `WifiManager` instance per application
- Handle both connected and disconnected states gracefully
- Consider battery usage with auto-reconnect settings

❌ **DON'T:**
- Call blocking operations in callbacks
- Use delays in callbacks
- Assume immediate connection after `begin()` (ESP32)
- Create multiple `WifiManager` instances unnecessarily
- Perform String operations in callbacks (heap fragmentation)
- Forget to handle disconnection scenarios
- Ignore connection status before network operations

## Performance Characteristics

- **Memory:** ~200-300 bytes per instance (platform-dependent)
- **Connection time:** 1-10 seconds (network-dependent)
- **Event latency (ESP32):** < 100 ms
- **Auto-reconnect delay (ESP32):** Immediate retry

## Example: Production-Ready Application

```cpp
#include "wifi_manager.h"

class MyApplication {
private:
    WifiManager wifi;
    bool networkReady = false;
    unsigned long lastConnectionAttempt = 0;
    
public:
    MyApplication() : wifi("MySSID", "MyPassword") {
        wifi.setLogger(Serial);
        wifi.setAutoReconnect(true);
        
        wifi.onConnected([this](String ip) {
            Serial.println("Connected: " + ip);
            Serial.println("Signal: " + String(wifi.strength()) + " dBm");
            networkReady = true;
            onNetworkAvailable();
        });
        
        wifi.onDisconnected([this]() {
            Serial.println("Disconnected");
            networkReady = false;
            onNetworkLost();
        });
    }
    
    void begin() {
        wifi.begin();
        lastConnectionAttempt = millis();
    }
    
    void loop() {
        if (networkReady) {
            doNetworkOperations();
        } else {
            doOfflineOperations();
        }
        
        // Monitor connection health
        if (networkReady) {
            long rssi = wifi.strength();
            if (rssi < -80) {
                Serial.println("Warning: Weak signal");
            }
        }
    }
    
private:
    void onNetworkAvailable() {
        // Start network services
        startWebServer();
        connectToCloud();
    }
    
    void onNetworkLost() {
        // Stop network services gracefully
        stopWebServer();
        disconnectFromCloud();
    }
    
    void doNetworkOperations() {
        // Upload data, serve web requests, etc.
    }
    
    void doOfflineOperations() {
        // Cache data, show offline status, etc.
    }
    
    void startWebServer() { /* ... */ }
    void stopWebServer() { /* ... */ }
    void connectToCloud() { /* ... */ }
    void disconnectFromCloud() { /* ... */ }
};

MyApplication app;

void setup() {
    Serial.begin(115200);
    app.begin();
}

void loop() {
    app.loop();
}
```

## Unit Tests

Reference comprehensive tests in `test/test_wifi_manager.cpp`:
- Constructor validation
- Connection lifecycle (both platforms)
- Event handling (ESP32)
- Callback registration and firing
- Auto-reconnect behavior
- Status transitions
- Edge cases and error handling
- Integration scenarios

To execute tests:
```bash
pio test -f test_wifi_manager
```

## License

This component is subject to the main project license (see `LICENSE`).

---

**Need help?** Check the examples above or review the test suite for detailed usage scenarios.
