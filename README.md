# Hub Robot Core

This is the base library for the Hub's low-powered Robot projects.

Primarily designed for supporting Arduino or ESP based robotic projects, this library provides a comprehensive collection of components and utilities to streamline embedded development.

See also: 

* [Hub Robot HTTP](https://github.com/demo-ninjas/hub-robot-http-utils) - Very basic HTTP Server for low powered robots
* [Hub Robot OLED](https://github.com/demo-ninjas/hub-robot-oled) - Simple OLED wrapper library to support some of the small OLED screens used in the HUB

## Available Components

See detailed usage guides for each component provided by this library: 

### **Button** - [Usage Guide](./docs/Button_UsageGuide.md)
Advanced button handling with debouncing, interrupt support, and multiple press types. Features single press, double press, long press detection with customizable timing. Supports both polling and interrupt modes for efficient operation.

**Key Features:**
- Hardware debouncing with configurable timing
- Multiple press types (single, double, long)
- Interrupt-driven or polling modes
- State tracking and duration measurement
- Memory efficient (72 bytes per instance)

### **DC Motor** - [Usage Guide](./docs/DCMotor_UsageGuide.md)
Simple DC motor control for H-bridge drivers (L298N, TB6612FNG, DRV8833). Provides signed speed control (-255 to +255), direction management, and active braking capability.

**Key Features:**
- Signed speed control with automatic clamping
- Forward, reverse, coast, and brake modes
- Redundancy optimization (skips unnecessary pin updates)
- Compatible with common H-bridge drivers
- Emergency brake functionality

### **Shift Register** - [Usage Guide](./docs/ShiftRegister_UsageGuide.md)
Control cascaded 74HC595 shift registers for GPIO expansion. Supports 1-8 cascaded chips (8-64 outputs) with MSB-first serial output, deferred updates for efficiency, and bounds checking.

**Key Features:**
- Support for 1-8 cascaded shift registers (up to 64 outputs)
- Batch updates for efficiency
- Individual or bulk output control
- Memory-safe with bounds checking
- ~50µs update time per register

## Available Utilities

### **WiFi Manager** - [Usage Guide](./docs/WiFiManager_UsageGuide.md)
Cross-platform WiFi connection management with event-driven callbacks. Handles automatic reconnection, connection state monitoring, and provides signal strength information. Optimized for ESP32 with fallback support for other platforms.

**Key Features:**
- Asynchronous connection handling (ESP32)
- Auto-reconnect functionality
- Connection state callbacks
- Signal strength monitoring
- Cross-platform compatibility (ESP32, WiFiNINA)

### **I2C Utils** - [Usage Guide](./docs/I2CUtils_UsageGuide.md)
Lightweight I2C bus scanning utilities for device discovery and debugging. Efficient address probing with customizable ranges, error reporting, and callback support for found devices.

**Key Features:**
- Efficient I2C bus scanning (20µs spacing)
- Customizable scan ranges
- Device discovery callbacks
- Support for multiple I2C buses
- Watchdog-friendly operation

### **String Utils** - [Usage Guide](./docs/StringUtils_UsageGuide.md)
Memory-efficient string manipulation utilities supporting both Arduino String and std::string. Includes UTF-8 character counting, string splitting with delimiter handling, and whitespace trimming.

**Key Features:**
- UTF-8 character counting and validation
- String splitting with configurable delimiters
- Whitespace trimming
- Support for Arduino String and std::string
- Memory-efficient with pre-allocation strategies

### **Serial Proxy** - [Usage Guide](./docs/SerialProxy_UsageGuide.md)
Print-compatible serial output mirror with circular buffering. Captures serial output in a ring buffer while forwarding to Serial, enabling programmatic access to recent log entries for remote diagnostics.

**Key Features:**
- Print interface compatibility
- Circular buffer for recent output
- Tail functionality for log retrieval
- Memory-efficient ring buffer design
- Emergency and preventive cleanup support

### **Memory Utils** - [Usage Guide](./docs/MemoryUtils_UsageGuide.md)
Real-time heap memory monitoring and management utilities. Provides platform-specific implementations for ESP32 with heap fragmentation detection and memory usage statistics.

**Key Features:**
- Real-time heap monitoring
- Fragmentation detection (ESP32)
- Memory usage statistics and trends
- Zero runtime overhead (inline functions)
- Platform-aware with graceful fallbacks

## Specific Hardware Components

The library also includes point in time copies of OSS support for the following specific hardware components:

### **ADS7828** - I2C 8-Channel 12-bit ADC
Located in `src/ads7828/`, provides interface for the ADS7828 I2C analog-to-digital converter.

### **LIS3DH** - 3-Axis Accelerometer  
Located in `src/LIS3DH/`, includes SparkFun's LIS3DH accelerometer library for motion sensing applications.

## Dev Machine Setup

Make sure you have the following installed on your machine: 

* A C/C++ compiler + standard dev tools installed (gcc, git, cmake etc...)
* PlatformIO Tools (https://platformio.org/)
* VSCode (https://code.visualstudio.com/)

Install the following VSCode Extensions: 

* C/C++ Extension pack (https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools-extension-pack) [This includes the C/C++ Extension]
* CMake Tools (https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools)
* PlatformIO IDE (https://marketplace.visualstudio.com/items?itemName=platformio.platformio-ide)

Install the `Arduino Espressif32` Framework in PlatformIO and make sure an ESP32 toolchain is installed (We typically use `ESP32S3`, eg. `toolchain-xtensa-esp32s3`).

## Quick Start Examples

### Basic Robot Control
```cpp
#include "button.h"
#include "dc_motor.h"
#include "wifi_manager.h"

Button startBtn(5);
DCMotor leftMotor(16, 17, 18);
DCMotor rightMotor(19, 20, 21);
WifiManager wifi("MyNetwork", "MyPassword");

void setup() {
    Serial.begin(115200);
    
    startBtn.onPressed([](long duration) {
        // Move forward when button pressed
        leftMotor.setSpeed(150);
        rightMotor.setSpeed(150);
    });
    
    wifi.begin();
}

void loop() {
    startBtn.tick();
}
```

### I2C Device Discovery
```cpp
#include <Wire.h>
#include "i2c_utils.hpp"

void setup() {
    Serial.begin(115200);
    Wire.begin();
    
    Serial.println("Scanning I2C bus...");
    int devices = scan_i2c();
    Serial.printf("Found %d devices\n", devices);
}
```

### Memory Monitoring
```cpp
#include "memory_utils.hpp"

void setup() {
    Serial.begin(115200);
    
    size_t free = freeRam();
    Serial.printf("Free memory: %u bytes\n", free);
    
    #if defined(ARDUINO_ARCH_ESP32)
    Serial.printf("Heap usage: %.1f%%\n", heapUsagePercent());
    #endif
}
```

## Library Architecture

This library follows several design principles:

- **Header-only utilities** for zero linking overhead where possible
- **Platform-aware implementations** with graceful fallbacks
- **Memory efficiency** optimized for embedded constraints  

## Performance Characteristics

| Component | Memory Usage | Update Time | Notes |
|-----------|--------------|-------------|-------|
| Button | ~72 bytes | <10µs | Per instance |
| DCMotor | ~24 bytes | <5µs | Per instance |
| ShiftRegister | ~24 bytes | ~50µs/register | Per update |
| WifiManager | ~200-300 bytes | N/A | Platform dependent (could be more) |
| String Utils | Minimal | O(n) | Header-only |
| Memory Utils | Minimal | <50µs | Header-only |


## Platform Support

- **ESP32** - Full support with all features
- **ESP8266** - Basic support, some limitations
- **Arduino AVR** (Uno, Mega) - Basic support, memory constraints
- **Arduino with WiFiNINA** - WiFi functionality supported
- **Other Arduino-compatible** - Core functionality available

## Publish

This library is intended to be used as a dependency for a Platform.io app.

The `library.json` describes the library and dependencies to Platform.io.

increment the version number when making changes you want to be used by dependent libraries.

Currently, not published to PlatformIO, load the library dependency directly via the GitHub URL.

## Contributing

Contributions are welcome! Please:

1. Follow the existing code style and patterns
2. Update documentation and usage guides
3. Test on multiple platforms where applicable
4. Follow memory-efficient design principles

## License

This project is licensed under the terms specified in the `LICENSE` file.

---

**Need help?** Check the detailed usage guides in the `docs/` folder for comprehensive examples and troubleshooting information.