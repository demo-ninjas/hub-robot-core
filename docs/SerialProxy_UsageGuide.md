# SerialProxy Class - Usage Guide

## Quick Start

```cpp
#include "serial_proxy.hpp"

SerialProxy proxy(1024); // 1KB ring buffer

void setup() {
    Serial.begin(115200);
    proxy.write("System starting...\n");
}

void loop() {
    proxy.write("Loop tick \n");
    delay(1000);

    // Retrieve last few lines
    std::string recent = proxy.tail(5);
    // You can forward this to another interface, or inspect for debugging
}
```

## Overview
`SerialProxy` is a lightweight Print-compatible helper that mirrors all output to the global `Serial` object while keeping a circular (ring) buffer of recent bytes. This allows you to:
- Inspect recent log output programmatically (e.g., send tail over WiFi / BLE / debug web page).
- Avoid large heap strings or growing `String` objects.
- Continue logging even if the consumer disconnects temporarily.

It implements the `Print` interface, enabling use anywhere an Arduino `Print*` is accepted.

## Constructor
```cpp
SerialProxy(size_t buf_size = 2048);
```
### Parameters
- `buf_size`: Capacity (bytes) of the internal ring buffer. Content wraps on overflow and always contains the most recent `buf_size` bytes.
  - Typical values: 512–4096 depending on RAM budget.
  - Minimum internally enforced as 1 to avoid zero allocation.

## Core API
```cpp
size_t write(uint8_t c) override;                  // Single byte (Print override)
size_t write(const uint8_t* buffer, size_t size) override; // Raw byte array (Print override)
size_t write(const char* str);                     // C-string
size_t write(const char* buffer, size_t size);     // Buffer + size
size_t write(const char* buffer, size_t size, size_t offset); // Sub-range of larger buffer
size_t write(const String& str);                   // Arduino String
void   flush() override;                           // Flush underlying Serial
std::string tail(size_t lines = 20) const override;// Last N newline-delimited lines
void   clear() override;                           // Reset ring buffer
size_t size() const;                               // Current stored bytes (<= capacity)
size_t capacity() const;                           // Configured capacity
bool   wrapped() const;                            // Has buffer wrapped at least once?
```

### Behavior Details
- All `write()` methods mirror output to `Serial` if initialized (i.e., after `Serial.begin`). If `Serial` is not ready, data is still buffered internally.
- Buffer overflow wraps automatically (ring semantics). Oldest data is overwritten first.
- `tail(lines)` scans backward looking for newline `\n` characters returning up to the requested line count; if fewer lines exist, returns what is available.
- No internal dynamic resizing occurs (predictable memory usage).

## Example Patterns
### 1. Unified Logger for Multiple Components
```cpp
SerialProxy logProxy(2048);

void setup() {
    Serial.begin(115200);
}

void loop() {
    logProxy.write("Sensor value: ");
    int val = analogRead(34);
    logProxy.write(String(val));
    logProxy.write("\n");
    delay(250);
}
```

### 2. Remote Diagnostics Over WiFi
```cpp
#include "wifi_manager.h"
SerialProxy logProxy(4096);
WiFiManager wifi("ssid", "pass");

void setup() {
    Serial.begin(115200);
    wifi.connect();
    logProxy.write("WiFi connecting...\n");
}

void loop() {
    if (wifi.isConnected()) {
        std::string recent = logProxy.tail(30);
        // send 'recent' over your custom protocol
    }
}
```

### 3. Conditional Serial Availability
```cpp
SerialProxy proxy(1024);

void setup() {
    // Intentionally delay Serial.begin based on condition
    if (digitalRead(0) == HIGH) {
        Serial.begin(115200); // If debugging jumper set
        proxy.write("Debug mode enabled\n");
    }
}

void loop() {
    proxy.write("tick\n"); // Captured even if Serial not started
}
```

### 4. Slicing Large Buffers
```cpp
const char* bigMsg = "HEADER:abc\nPayload line 1\nPayload line 2\n";
SerialProxy proxy(512);

void setup() {
    Serial.begin(115200);
    proxy.write(bigMsg, 7, 0);   // Writes "HEADER:"
    proxy.write(bigMsg, strlen(bigMsg) - 7, 7); // Writes remainder
}
```

### 5. Clearing Logs After Transmission
```cpp
SerialProxy proxy(2048);

void loop() {
    // ... log writes ...
    std::string last = proxy.tail(50);
    // transmit last
    proxy.clear(); // Start fresh after successful send
}
```

## Best Practices
✅ Choose a buffer size that balances RAM usage and diagnostic needs.
✅ Call `tail()` only when needed (it performs a reverse scan; keep infrequent for large buffers).
✅ Use `size()` and `wrapped()` to decide whether to stream entire history or just tail.
✅ Reuse the same instance globally; avoid repeatedly constructing/destroying (prevents fragmentation).
✅ Prefer `write(const char*, size, offset)` for partial sends from static buffers to avoid temporary String creation.

❌ Avoid very large buffers (>8192) on memory-constrained MCUs unless justified.
❌ Don’t rely on `tail()` for parsing protocol state (it’s for diagnostics, not a guaranteed message boundary handler).
❌ Don’t create multiple `SerialProxy` instances that all mirror to `Serial` without clear ownership (can duplicate output needlessly).

## Memory & Performance
- Write operations are O(1) per byte (simple index + assignment).
- `tail()` complexity: O(N) worst case (N = buffer capacity) when seeking many lines. For typical small line counts and moderate buffer sizes this is acceptable.
- No heap growth after construction; uses a single `new[]` allocation.
- If allocation fails (`buf_ == nullptr`), writes degrade gracefully: data still goes to `Serial` but tail/clear operate as no-ops.

## Edge Cases
| Scenario | Result |
|----------|--------|
| `buf_size = 0` | Internally coerced to 1 byte capacity |
| Allocation fails | tail() returns empty; writes still forwarded to Serial |
| `write(nullptr, size)` | Ignored (returns 0) |
| `write(buffer, 0)` | No-op |
| `write(buffer, size, offset)` offset beyond length | Returns 0 (nothing written) |
| `tail(0)` | Empty string |
| `tail(lines)` when fewer lines exist | Returns all available |

## Retrieving Tail Without Newlines
If your output does not include newline delimiters, `tail()` will return up to buffer capacity in reverse scan order effectively giving recent bytes. Consider adding newlines for clearer segmentation.

## Integration With Existing Print-based APIs
Because `SerialProxy` inherits from `Print`, you can pass it to APIs expecting a `Print&`:
```cpp
void dumpStatus(Print& out) {
    out.print("Status: OK\n");
}

SerialProxy proxy(1024);

void loop() {
    dumpStatus(proxy); // Mirrors to Serial and caches in ring buffer
}
```

## Troubleshooting
| Issue | Check |
|-------|-------|
| tail() always empty | Ensure at least one write occurred; verify allocation (maybe low RAM). |
| Old data missing | Expected on wrap; increase buffer size if history needed. |
| Serial output missing | Confirm `Serial.begin()` called and baud matches monitor. |
| Memory pressure | Reduce buffer size or share instance; avoid huge debug strings. |

## Example: Periodic Diagnostic Dump
```cpp
SerialProxy proxy(3072);
unsigned long lastDump = 0;

void loop() {
    proxy.write("Heartbeat \n");
    if (millis() - lastDump > 5000) {
        lastDump = millis();
        std::string recent = proxy.tail(25);
        Serial.println("==== Recent Log ====");
        Serial.print(recent.c_str());
        Serial.println("====================");
    }
    delay(250);
}
```

## License
This component is subject to the main project license (see `LICENSE`).

---
Contributions & improvements welcome! Feel free to extend with filtering or timestamping.
