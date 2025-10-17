````markdown
# I2C Utilities - Usage Guide

## Quick Start

```cpp
#include <Wire.h>
#include "i2c_utils.hpp"

void setup() {
    Serial.begin(115200);
    Wire.begin();              // Initialize default I2C bus

    // Basic scan (default range 0x08-0x77)
    int found = scan_i2c();
    Serial.print("Devices found: ");
    Serial.println(found);
}

void loop() {
    // Application code
}
```

## Overview

The I2C utilities provide a lightweight, header-only helper for scanning the I2C bus to discover connected devices. This is especially useful during development and debugging to confirm wiring, address configuration (jumpers, solder bridges), and presence of sensors/expanders.

**Key Features:**
- Efficient address probing (default ~20µs spacing)
- Customizable scan range (start/end address)
- Optional error reporting (Wire error code 4)
- Callback for each discovered device
- Supports alternative I2C buses (e.g., `Wire1` on ESP32 / RP2040)
- Safe defaults (skips reserved low addresses by default)
- Watchdog-friendly (periodic `yield()` for long scans)

## API Reference

### `scan_i2c()`

```cpp
int scan_i2c(
    Print* printer = nullptr,
    TwoWire& wire = Wire,
    uint8_t startAddress = 0x08,
    uint8_t endAddress   = 0x77,
    uint16_t delayMicros = 20,
    bool showErrors = false,
    void (*foundCallback)(uint8_t address) = nullptr
);
```

#### Parameters
- **printer**: Optional output target (defaults to `Serial` if available). Pass `nullptr` for silent scan.
- **wire**: The I2C bus instance to scan. Default is global `Wire`. Use `Wire1` or custom `TwoWire` for secondary buses.
- **startAddress**: First 7-bit I2C address to test (inclusive). Default `0x08` (skip reserved region 0x00–0x07).
- **endAddress**: Last 7-bit I2C address (inclusive). Default `0x77` (max valid 7-bit address).
- **delayMicros**: Microseconds to wait between probes. Increase if dealing with slow or heavily loaded buses.
- **showErrors**: If `true`, prints addresses that return Wire error code 4 (other non-ACK responses ignored).
- **foundCallback**: Optional function pointer invoked for each found address. Signature: `void callback(uint8_t address)`.

#### Returns
- Number of devices that acknowledged during the scan.

#### Example: Custom Range & Callback
```cpp
#include "i2c_utils.hpp"

void onFound(uint8_t addr) {
    Serial.print("Callback: device at 0x");
    if (addr < 16) Serial.print('0');
    Serial.println(addr, HEX);
}

void setup() {
    Serial.begin(115200);
    Wire.begin();

    int count = scan_i2c(&Serial, Wire, 0x10, 0x5F, 50, true, onFound);
    Serial.printf("Scan finished: %d device(s)\n", count);
}
```

## Common Usage Patterns

### Pattern 1: Silent Scan (No Output)
```cpp
Wire.begin();
int devices = scan_i2c(nullptr);  // No printing
if (devices == 0) {
    // Take corrective action (reinitialize bus, log, etc.)
}
```

### Pattern 2: Secondary Bus (ESP32)
```cpp
TwoWire I2C_EXT = TwoWire(1);  // Use Wire instance 1

void setup() {
    Serial.begin(115200);
    I2C_EXT.begin(21, 22, 400000);  // SDA=21, SCL=22, 400kHz
    scan_i2c(&Serial, I2C_EXT);     // Scan external bus
}
```

### Pattern 3: Fast Diagnostic Button
```cpp
#include "button.h"  // Assuming existing button helper
#include "i2c_utils.hpp"

Button diagBtn(4, true);  // Pull-up button on GPIO4

void loop() {
    if (diagBtn.wasPressed()) {
        scan_i2c();  // Print current bus devices
    }
}
```

### Pattern 4: Auto-Recover If Device Missing
```cpp
uint8_t REQUIRED_ADDR = 0x3C;  // Example OLED

bool ensureDevicePresent() {
    bool present = false;
    scan_i2c(nullptr, Wire, REQUIRED_ADDR, REQUIRED_ADDR, 40, false, [&](uint8_t addr){ present = true; });
    if (!present) {
        // Try lowering bus speed or reinitializing
        Wire.end();
        delay(10);
        Wire.begin();
    }
    return present;
}
```

### Pattern 5: Log to Alternate Output
```cpp
#include <HardwareSerial.h>
HardwareSerial DebugSerial(1);

void setup() {
    DebugSerial.begin(115200, SERIAL_8N1, 16, 17); // RX=16, TX=17
    Wire.begin();
    scan_i2c(&DebugSerial);  // Send output to external debug UART
}
```

## Performance Notes

- Default scan (0x08–0x77) probes 112 addresses.
- With 20µs delay + bus transaction time, typical ESP32 full scan: ~15–25 ms.
- Increasing `delayMicros` improves compatibility with marginal devices but slows total scan time.
- A `yield()` occurs periodically to avoid watchdog resets on long ranges or slower boards.

### Optimization Tips
✅ Restrict address range if you know probable device addresses.
```cpp
scan_i2c(&Serial, Wire, 0x40, 0x4F);  // Only probe 16 addresses
```
✅ Keep delay low (10–30µs) unless diagnosing timing-sensitive hardware.
✅ Use callback instead of parsing textual output if machine-to-machine.

## Error Handling

- Error code `4` (often "unknown error") is optionally printed when `showErrors=true`.
- Other non-zero error codes usually indicate a NACK (no device); these are silently ignored for speed.
- Invalid range (`startAddress > endAddress`) returns `0` immediately.

## Reserved Addresses

Addresses below `0x08` are generally reserved (e.g., general call, CBUS). The default start address avoids probing these to prevent unintended side effects.
If you need to probe them for a specialized application, explicitly set `startAddress = 0x00`.

```cpp
// Full raw sweep (use with caution)
scan_i2c(&Serial, Wire, 0x00, 0x77);
```

## When to Use Scanning

✅ Initial hardware bring-up
✅ Verifying solder bridges / address jumpers
✅ Detecting presence/absence of optional modules
✅ Debugging unexpected bus failures

❌ Not recommended in a time-critical loop (avoid repeated full scans every frame)
❌ Not a substitute for device-level initialization success checks

## Integration Example: Conditional Sensor Init
```cpp
uint8_t TMP117_ADDR = 0x48;

bool sensorAvailable = false;
scan_i2c(nullptr, Wire, TMP117_ADDR, TMP117_ADDR, 25, false, [&](uint8_t){ sensorAvailable = true; });

if (sensorAvailable) {
    if (initTmp117()) {
        Serial.println("TMP117 initialized OK");
    } else {
        Serial.println("TMP117 responded but init failed");
    }
} else {
    Serial.println("TMP117 not detected on bus");
}
```

## Advanced: Multiple Buses & Deferred Logging
```cpp
struct BusScanResult {
    uint8_t addresses[16];
    uint8_t count = 0;
};

BusScanResult scanResult;

void collect(uint8_t addr) {
    if (scanResult.count < sizeof(scanResult.addresses)) {
        scanResult.addresses[scanResult.count++] = addr;
    }
}

void setup() {
    Serial.begin(115200);
    Wire.begin();

    // Collect silently
    scanResult.count = 0;
    scan_i2c(nullptr, Wire, 0x08, 0x77, 15, false, collect);

    // Print summary
    Serial.print("Found "); Serial.print(scanResult.count); Serial.println(" device(s):");
    for (uint8_t i = 0; i < scanResult.count; ++i) {
        Serial.print("  0x"); if (scanResult.addresses[i] < 16) Serial.print('0');
        Serial.println(scanResult.addresses[i], HEX);
    }
}
```

## Troubleshooting

| Issue | Possible Causes | Actions |
|-------|-----------------|---------|
| No devices found | Wiring error, wrong pins, pull-ups missing | Verify SDA/SCL, add 4.7k pull-ups, check voltage levels |
| Devices intermittent | Loose connections, bus speed too high | Reduce clock (e.g., 100kHz), re-seat connectors |
| Unexpected addresses | Address jumpers mis-set | Check datasheet & physical jumpers |
| Scan freezes | Long blocking operations elsewhere | Ensure `yield()` is not starved, avoid huge delays in ISRs |
| Error 4 printed | Electrical noise, bus contention | Simplify bus, shorten wires, add decoupling |

## Best Practices

✅ Initialize the bus once (`Wire.begin()`) before scanning.
✅ Keep scan usage diagnostic; avoid frequent scans in production loops.
✅ Log addresses in hex AND decimal for clarity during debugging.
✅ Restrict range for faster targeted scans.
✅ Use callback for structured data collection.

❌ Do not assume scan success guarantees functional device initialization.
❌ Avoid scanning while time-critical I2C transactions are in progress (can add latency).
❌ Do not probe reserved addresses unless explicitly required.

## Example: Periodic Health Check (Non-Blocking)
```cpp
unsigned long lastScan = 0;
const unsigned long SCAN_INTERVAL_MS = 60000; // 1 minute

void loop() {
    if (millis() - lastScan > SCAN_INTERVAL_MS) {
        lastScan = millis();
        int found = scan_i2c(nullptr, Wire, 0x40, 0x4F, 15); // Narrow range for known devices
        if (found == 0) {
            Serial.println("Warning: Expected I2C range empty");
        }
    }

    // Rest of application logic
}
```

## Unit Testing

You can wrap the scan in a test harness (mock TwoWire / simulate acknowledgments) to validate logic without hardware. In production environments, integration tests using known device addresses are recommended.

Run all project tests:
```bash
pio test
```

## License

This component follows the main project license (see `LICENSE`).

---
**Need help?** Re-run `scan_i2c()` with `showErrors=true` and examine wiring & pull-ups.
````