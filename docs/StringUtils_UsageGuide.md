````markdown
# String Utilities - Usage Guide

## Quick Start

```cpp
#include "string_utils.hpp"

void setup() {
    Serial.begin(115200);
    
    // Split a string
    String csv = "sensor1,sensor2,sensor3";
    std::vector<String> sensors = split(csv, ',');
    
    // Count UTF-8 characters
    String emoji = "Hello 👋 World 🌍";
    Serial.print("Character count: ");
    Serial.println(utf8CharCount(emoji));
    
    // Trim whitespace
    String padded = "  data  ";
    String clean = trim(padded);  // "data"
}
```

## Overview

The String Utilities library provides a lightweight, header-only collection of string manipulation functions optimized for embedded systems. It supports both Arduino `String` and C++ `std::string` types, making it versatile for various use cases on ESP32 and other Arduino-compatible platforms.

**Key Features:**
- UTF-8 character counting and validation
- String splitting with configurable delimiter handling
- Whitespace trimming
- Memory-efficient implementations with pre-allocation
- No dynamic library linking (header-only)
- Zero-copy operations where possible
- Safe for use in memory-constrained environments

## Namespace

All functions are in the `HubStringUtils` namespace, with convenience `using` declarations at the bottom of the header for unqualified access.

```cpp
// Option 1: Use namespace (recommended for clarity)
#include "string_utils.hpp"
size_t count = HubStringUtils::utf8CharCount(str);

// Option 2: Unqualified (convenience using declarations included)
#include "string_utils.hpp"
size_t count = utf8CharCount(str);  // Works due to using declaration
```

## API Reference

### `utf8CharCount()` - Count UTF-8 Characters

```cpp
size_t utf8CharCount(const String& str) noexcept;
```

Counts the number of UTF-8 characters (code points) in an Arduino String, not bytes. Multi-byte sequences like emojis count as single characters.

**Parameters:**
- `str`: The Arduino String to analyze

**Returns:** Number of UTF-8 characters (not bytes)

**Performance:** O(n) where n is byte length

**Example:**

```cpp
String ascii = "Hello";
Serial.println(utf8CharCount(ascii));  // 5

String unicode = "Hello 👋";  // Wave emoji is 4 bytes
Serial.println(unicode.length());      // 10 (bytes)
Serial.println(utf8CharCount(unicode)); // 7 (characters)

String chinese = "你好世界";  // Each character is 3 bytes
Serial.println(chinese.length());       // 12 (bytes)
Serial.println(utf8CharCount(chinese)); // 4 (characters)
```

**Use Cases:**
- Display truncation (limit visible characters, not bytes)
- Text field validation (character limits)
- Progress indicators for text processing
- Protocol implementations requiring character counts

**Notes:**
- Invalid UTF-8 sequences are counted byte-by-byte
- Continuation bytes (0x80-0xBF) are not counted
- Safe for all input (no crashes on malformed UTF-8)
- Does not normalize or validate UTF-8 correctness

### `utf8ByteLength()` - Get UTF-8 Byte Length

```cpp
size_t utf8ByteLength(const String& str) noexcept;
```

Returns the byte length of the UTF-8 representation. For Arduino Strings (which are already UTF-8 encoded), this is equivalent to `str.length()`.

**Parameters:**
- `str`: The Arduino String to measure

**Returns:** Number of bytes in UTF-8 representation

**Example:**

```cpp
String text = "Hello 🌍";
Serial.println(utf8ByteLength(text));  // 10 (bytes)
Serial.println(text.length());         // 10 (same - already UTF-8)
```

**Use Cases:**
- Buffer allocation for transmission
- Protocol packet size calculation
- Storage size estimation
- Validation that String is properly UTF-8 encoded

**Notes:**
- Arduino Strings are stored as UTF-8, so this equals `.length()`
- Provided for API completeness and clarity of intent
- Useful when interfacing with systems that distinguish encoding

### `split()` - Split String by Delimiter

Two overloads: one for `std::string`, one for Arduino `String`.

#### std::string Overload

```cpp
std::vector<std::string> split(const std::string& str, 
                               char delimiter, 
                               bool keep_empty = false);
```

**Parameters:**
- `str`: The std::string to split
- `delimiter`: Single character delimiter
- `keep_empty`: If `true`, keep empty tokens from consecutive delimiters (default: `false`)

**Returns:** Vector of std::string tokens

**Example:**

```cpp
#include <string>

std::string csv = "apple,banana,cherry";
std::vector<std::string> fruits = split(csv, ',');
// fruits = ["apple", "banana", "cherry"]

// Handle consecutive delimiters
std::string data = "a,,b,c";
auto tokens1 = split(data, ',');        // ["a", "b", "c"] - skips empty
auto tokens2 = split(data, ',', true);  // ["a", "", "b", "c"] - keeps empty

// Leading/trailing delimiters
std::string path = "/home/user/";
auto dirs1 = split(path, '/');         // ["home", "user"] - skips empty
auto dirs2 = split(path, '/', true);   // ["", "home", "user", ""] - keeps empty
```

#### Arduino String Overload

```cpp
std::vector<String> split(const String& str, 
                         char delimiter, 
                         bool keep_empty = false);
```

**Parameters:**
- `str`: The Arduino String to split
- `delimiter`: Single character delimiter
- `keep_empty`: If `true`, keep empty tokens from consecutive delimiters (default: `false`)

**Returns:** Vector of Arduino String tokens

**Example:**

```cpp
String csv = "red,green,blue";
std::vector<String> colors = split(csv, ',');
// colors = ["red", "green", "blue"]

// Process each token
for (const String& color : colors) {
    Serial.println(color);
}

// CSV with empty fields
String record = "John,,Doe,30";
auto fields = split(record, ',', true);
// fields = ["John", "", "Doe", "30"]
```

**Performance:**
- Pre-reserves vector capacity to minimize reallocations
- Pre-reserves token string capacity (32 bytes initial)
- Uses `std::move` to avoid unnecessary copies (std::string version)
- O(n) time complexity where n is string length

**Use Cases:**
- CSV/TSV parsing
- Command parsing (e.g., "CMD:ARG1:ARG2")
- Path manipulation
- Configuration file parsing
- Protocol message tokenization

**Notes:**
- Empty input returns empty vector
- Single delimiter character (not substring)
- For multi-character delimiters, iterate and split repeatedly
- Memory efficient on ESP32 with typical strings (<1KB)

### `trim()` - Remove Whitespace

Two overloads: one for `std::string`, one for Arduino `String`.

#### std::string Overload

```cpp
std::string trim(const std::string& str);
```

**Parameters:**
- `str`: The std::string to trim

**Returns:** New std::string with leading/trailing whitespace removed

**Whitespace Characters:** space, tab (`\t`), newline (`\n`), carriage return (`\r`), form feed (`\f`), vertical tab (`\v`)

**Example:**

```cpp
#include <string>

std::string padded = "  hello world  ";
std::string clean = trim(padded);
// clean = "hello world"

std::string multiline = "\n\ttext\r\n";
std::string trimmed = trim(multiline);
// trimmed = "text"

std::string allSpaces = "    ";
std::string empty = trim(allSpaces);
// empty = ""
```

#### Arduino String Overload

```cpp
String trim(const String& str);
```

**Parameters:**
- `str`: The Arduino String to trim

**Returns:** New Arduino String with leading/trailing whitespace removed

**Whitespace Characters:** space, tab (`\t`), newline (`\n`), carriage return (`\r`)

**Example:**

```cpp
String userInput = "  sensor_data  \n";
String cleaned = trim(userInput);
// cleaned = "sensor_data"

// Processing user input
void handleCommand(String raw) {
    String cmd = trim(raw);
    if (cmd == "START") {
        // Process command
    }
}

// Clean configuration values
String configValue = "  192.168.1.100  ";
String ipAddress = trim(configValue);
// ipAddress = "192.168.1.100"
```

**Use Cases:**
- User input sanitization
- Configuration file parsing
- Serial command processing
- Comparing trimmed strings
- Preparing data for display

**Notes:**
- Does not modify original string
- Returns empty string if input is all whitespace
- Internal whitespace is preserved (only leading/trailing removed)
- Efficient implementation (single pass)

## Common Usage Patterns

### Pattern 1: CSV Parsing

```cpp
void parseCSV(String csvLine) {
    std::vector<String> fields = split(csvLine, ',');
    
    if (fields.size() >= 3) {
        String timestamp = trim(fields[0]);
        float temperature = trim(fields[1]).toFloat();
        int humidity = trim(fields[2]).toInt();
        
        Serial.printf("Time: %s, Temp: %.1f°C, Humidity: %d%%\n",
                      timestamp.c_str(), temperature, humidity);
    }
}

void setup() {
    Serial.begin(115200);
    parseCSV("2024-10-17 14:30:00, 23.5, 65");
}
```

### Pattern 2: Command Parser

```cpp
struct Command {
    String name;
    std::vector<String> args;
};

Command parseCommand(String input) {
    Command cmd;
    String cleaned = trim(input);
    
    std::vector<String> parts = split(cleaned, ' ');
    if (!parts.empty()) {
        cmd.name = parts[0];
        cmd.args = std::vector<String>(parts.begin() + 1, parts.end());
    }
    
    return cmd;
}

void processCommand(String raw) {
    Command cmd = parseCommand(raw);
    
    if (cmd.name == "SET") {
        if (cmd.args.size() >= 2) {
            String key = cmd.args[0];
            String value = cmd.args[1];
            // Process SET key value
        }
    } else if (cmd.name == "GET") {
        if (cmd.args.size() >= 1) {
            String key = cmd.args[0];
            // Process GET key
        }
    }
}

void loop() {
    if (Serial.available()) {
        String input = Serial.readStringUntil('\n');
        processCommand(input);  // "SET temperature 25"
    }
}
```

### Pattern 3: Multi-Level Parsing

```cpp
void parsePath(String path) {
    // Split by '/' for directories, then by '.' for file extension
    std::vector<String> parts = split(path, '/');
    
    for (const String& part : parts) {
        if (part.indexOf('.') >= 0) {
            // This is a file
            std::vector<String> fileparts = split(part, '.');
            String name = fileparts[0];
            String ext = fileparts.size() > 1 ? fileparts[1] : "";
            Serial.printf("File: %s, Extension: %s\n", 
                         name.c_str(), ext.c_str());
        } else {
            // This is a directory
            Serial.printf("Directory: %s\n", part.c_str());
        }
    }
}

void setup() {
    Serial.begin(115200);
    parsePath("data/sensors/temperature.csv");
}
```

### Pattern 4: Configuration File Parsing

```cpp
struct Config {
    String key;
    String value;
};

std::vector<Config> parseConfig(const String& configData) {
    std::vector<Config> configs;
    
    // Split by newlines
    std::vector<String> lines = split(configData, '\n');
    
    for (String line : lines) {
        line = trim(line);
        
        // Skip comments and empty lines
        if (line.length() == 0 || line.startsWith("#")) {
            continue;
        }
        
        // Split by '='
        std::vector<String> pair = split(line, '=');
        if (pair.size() == 2) {
            Config cfg;
            cfg.key = trim(pair[0]);
            cfg.value = trim(pair[1]);
            configs.push_back(cfg);
        }
    }
    
    return configs;
}

void setup() {
    String configText = 
        "# Network Configuration\n"
        "ssid = MyNetwork\n"
        "password = SecurePass123\n"
        "# MQTT Settings\n"
        "mqtt_server = broker.example.com\n";
    
    auto configs = parseConfig(configText);
    
    for (const auto& cfg : configs) {
        Serial.printf("%s = %s\n", cfg.key.c_str(), cfg.value.c_str());
    }
}
```

### Pattern 5: Display Text Truncation

```cpp
String truncateForDisplay(String text, size_t maxChars, bool addEllipsis = true) {
    size_t charCount = utf8CharCount(text);
    
    if (charCount <= maxChars) {
        return text;
    }
    
    // Truncate by walking through and counting characters
    size_t bytePos = 0;
    size_t currentChars = 0;
    
    while (bytePos < text.length() && currentChars < maxChars) {
        unsigned char c = text[bytePos];
        
        // Count only start bytes (not continuation bytes)
        if ((c & 0xC0) != 0x80) {
            ++currentChars;
        }
        ++bytePos;
    }
    
    String result = text.substring(0, bytePos);
    if (addEllipsis) {
        result += "...";
    }
    
    return result;
}

void displayOnLCD(String message, int maxWidth) {
    String truncated = truncateForDisplay(message, maxWidth);
    // Display on LCD...
}
```

### Pattern 6: Protocol Message Parsing

```cpp
struct SensorMessage {
    String sensorId;
    String dataType;
    float value;
    bool valid = false;
};

SensorMessage parseMessage(String message) {
    SensorMessage msg;
    
    // Protocol: "SENSOR:ID:TYPE:VALUE"
    std::vector<String> parts = split(message, ':');
    
    if (parts.size() == 4 && trim(parts[0]) == "SENSOR") {
        msg.sensorId = trim(parts[1]);
        msg.dataType = trim(parts[2]);
        msg.value = trim(parts[3]).toFloat();
        msg.valid = true;
    }
    
    return msg;
}

void loop() {
    if (Serial.available()) {
        String raw = Serial.readStringUntil('\n');
        SensorMessage msg = parseMessage(raw);
        
        if (msg.valid) {
            Serial.printf("Sensor %s reported %s: %.2f\n",
                         msg.sensorId.c_str(),
                         msg.dataType.c_str(),
                         msg.value);
        }
    }
}
```

### Pattern 7: Query String Parsing

```cpp
struct QueryParam {
    String key;
    String value;
};

std::vector<QueryParam> parseQueryString(String query) {
    std::vector<QueryParam> params;
    
    // Remove leading '?' if present
    if (query.startsWith("?")) {
        query = query.substring(1);
    }
    
    // Split by '&'
    std::vector<String> pairs = split(query, '&');
    
    for (const String& pair : pairs) {
        std::vector<String> kv = split(pair, '=');
        if (kv.size() == 2) {
            QueryParam param;
            param.key = trim(kv[0]);
            param.value = trim(kv[1]);
            params.push_back(param);
        }
    }
    
    return params;
}

void handleRequest(String url) {
    int queryPos = url.indexOf('?');
    if (queryPos >= 0) {
        String queryString = url.substring(queryPos);
        auto params = parseQueryString(queryString);
        
        for (const auto& param : params) {
            Serial.printf("Param: %s = %s\n", 
                         param.key.c_str(), 
                         param.value.c_str());
        }
    }
}

// Example: handleRequest("/api/sensor?id=temp01&format=json")
```

## Memory Considerations

### Stack vs Heap Usage

All functions return by value, which may involve heap allocation for vectors and strings:

```cpp
// This allocates on heap (vector + strings)
std::vector<String> tokens = split(longString, ',');

// For large datasets, process incrementally:
void processLargeCSV(String data) {
    std::vector<String> lines = split(data, '\n');
    
    for (const String& line : lines) {
        std::vector<String> fields = split(line, ',');
        processFields(fields);
        // fields vector freed after each iteration
    }
}
```

### Pre-allocation Strategy

The library pre-allocates to minimize reallocations:

```cpp
// split() pre-reserves:
tokens.reserve(str.length() / 10 + 1);  // Estimated token count
token.reserve(32);                       // Typical token size
```

### Memory Efficiency Tips

✅ **DO:**
- Process and discard tokens incrementally
- Use references to avoid copies: `for (const String& token : tokens)`
- Clear vectors when done: `tokens.clear(); tokens.shrink_to_fit();`
- Limit input size for embedded systems

❌ **DON'T:**
- Keep large token vectors in memory unnecessarily
- Split very large strings (>10KB) without streaming
- Create unnecessary intermediate copies

## Performance Characteristics

| Function | Time Complexity | Space Complexity | Notes |
|----------|----------------|------------------|-------|
| `utf8CharCount()` | O(n) | O(1) | Single pass, no allocations |
| `utf8ByteLength()` | O(1) | O(1) | Just returns length |
| `split()` | O(n) | O(k) | k = number of tokens |
| `trim()` | O(n) | O(m) | m = trimmed string length |

**Benchmark Results (ESP32 @ 240MHz):**

```cpp
// Typical performance (indicative, not guaranteed):
utf8CharCount("100 char string") : ~8 µs
split("10 tokens", ',')          : ~45 µs
trim("  text  ")                 : ~3 µs
```

## UTF-8 Handling Details

### UTF-8 Encoding Basics

UTF-8 uses 1-4 bytes per character:

| Character Range | Byte Pattern | Example |
|----------------|--------------|---------|
| U+0000 to U+007F | 0xxxxxxx | ASCII: 'A' |
| U+0080 to U+07FF | 110xxxxx 10xxxxxx | 'é' |
| U+0800 to U+FFFF | 1110xxxx 10xxxxxx 10xxxxxx | '你' |
| U+10000 to U+10FFFF | 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx | '🌍' |

### Character Counting Examples

```cpp
String s1 = "A";           // 1 byte, 1 character
String s2 = "é";           // 2 bytes, 1 character
String s3 = "你";          // 3 bytes, 1 character
String s4 = "🌍";          // 4 bytes, 1 character

Serial.println(utf8CharCount(s1));  // 1
Serial.println(utf8CharCount(s2));  // 1
Serial.println(utf8CharCount(s3));  // 1
Serial.println(utf8CharCount(s4));  // 1

String mixed = "A é 你 🌍";
Serial.println(mixed.length());          // 15 bytes
Serial.println(utf8CharCount(mixed));    // 7 characters (4 letters + 3 spaces)
```

### Invalid UTF-8 Handling

The library is defensive against malformed UTF-8:

```cpp
// Malformed UTF-8 (invalid continuation byte)
String bad = "A\x80";  // 0x80 is continuation byte without start byte

// utf8CharCount handles gracefully:
// - Counts 'A' as 1 character
// - Counts 0x80 as 1 character (invalid but counted)
size_t count = utf8CharCount(bad);  // 2 (defensive counting)
```

## Error Handling

The library uses defensive programming:

✅ **Safe Operations:**
- Empty string input returns appropriate empty result
- Invalid UTF-8 won't crash (counted defensively)
- All functions are `noexcept` where possible
- No null pointer dereferences

**Example Error Cases:**

```cpp
// Empty input
auto tokens = split(String(""), ',');  // Returns empty vector
size_t count = utf8CharCount("");      // Returns 0
String result = trim("");              // Returns empty string

// All whitespace
String spaces = "    ";
String trimmed = trim(spaces);         // Returns empty string

// No delimiters
auto tokens = split("no-commas", ','); // Returns vector with 1 element

// Consecutive delimiters
auto tokens = split("a,,b", ',');      // Returns ["a", "b"] (skips empty)
auto tokens = split("a,,b", ',', true); // Returns ["a", "", "b"] (keeps empty)
```

## Best Practices

### ✅ DO:

- Use `const String&` in your functions accepting strings
- Pre-allocate when you know expected size
- Process tokens incrementally for large data
- Use `keep_empty=true` when empty fields are meaningful (CSV with optional columns)
- Validate input sizes for embedded constraints
- Use references in range-based for loops: `for (const String& token : tokens)`
- Clear vectors when done with large datasets

### ❌ DON'T:

- Split multi-megabyte strings on embedded systems
- Keep large token vectors in memory longer than needed
- Assume UTF-8 validity without validation in security-critical code
- Use string concatenation in tight loops (use token iteration instead)
- Forget that `split()` returns by value (copies data)

## Integration with Other Libraries

### With WiFiManager

```cpp
#include "string_utils.hpp"
#include "wifi_manager.h"

void parseCredentials(String config) {
    // Config format: "SSID:PASSWORD"
    std::vector<String> parts = split(config, ':');
    
    if (parts.size() == 2) {
        String ssid = trim(parts[0]);
        String pass = trim(parts[1]);
        
        WifiManager wifi(ssid, pass);
        wifi.begin();
    }
}
```

### With Serial Communication

```cpp
#include "string_utils.hpp"

void setup() {
    Serial.begin(115200);
}

void loop() {
    if (Serial.available()) {
        String input = Serial.readStringUntil('\n');
        input = trim(input);
        
        std::vector<String> args = split(input, ' ');
        
        if (!args.empty()) {
            String command = args[0];
            // Process command...
        }
    }
}
```

### With JSON Parsing (Simple)

```cpp
#include "string_utils.hpp"

// Simple JSON object parser (for basic cases)
void parseSimpleJSON(String json) {
    // Remove braces
    json = trim(json);
    if (json.startsWith("{")) json = json.substring(1);
    if (json.endsWith("}")) json = json.substring(0, json.length() - 1);
    
    // Split by commas
    std::vector<String> pairs = split(json, ',');
    
    for (String pair : pairs) {
        // Split by colon
        std::vector<String> kv = split(pair, ':');
        if (kv.size() == 2) {
            String key = trim(kv[0]);
            String value = trim(kv[1]);
            
            // Remove quotes
            key.replace("\"", "");
            value.replace("\"", "");
            
            Serial.printf("%s = %s\n", key.c_str(), value.c_str());
        }
    }
}

// Example: parseSimpleJSON("{\"temp\":23.5, \"humidity\":65}")
```

## Troubleshooting

### Issue: Character count seems wrong

**Symptoms:** `utf8CharCount()` returns unexpected value

**Causes:**
1. String contains multi-byte UTF-8 characters
2. Comparing with `.length()` (which returns bytes)
3. Invalid UTF-8 encoding

**Solutions:**
```cpp
String text = "Hello 👋";
Serial.println(text.length());        // 10 (bytes)
Serial.println(utf8CharCount(text));  // 7 (characters)
// This is correct! The emoji is 4 bytes but 1 character
```

### Issue: Split returns unexpected tokens

**Symptoms:** Missing tokens or extra empty tokens

**Causes:**
1. Consecutive delimiters
2. Leading/trailing delimiters
3. Not using `keep_empty` parameter correctly

**Solutions:**
```cpp
String data = "a,,b";

// If you want ["a", "b"]:
auto tokens = split(data, ',');  // Default: skip empty

// If you want ["a", "", "b"]:
auto tokens = split(data, ',', true);  // Keep empty
```

### Issue: Memory errors with large strings

**Symptoms:** Crashes, heap allocation failures

**Causes:**
1. Splitting very large strings
2. Keeping too many token vectors in memory
3. Fragmented heap

**Solutions:**
```cpp
// BAD: Split entire large file at once
// std::vector<String> allLines = split(hugeFile, '\n');

// GOOD: Process incrementally
void processLargeFile(String data) {
    int start = 0;
    int end = data.indexOf('\n');
    
    while (end >= 0) {
        String line = data.substring(start, end);
        processLine(line);  // Process one line at a time
        
        start = end + 1;
        end = data.indexOf('\n', start);
    }
}
```

### Issue: Trim doesn't remove all whitespace

**Symptoms:** Whitespace still present after trim

**Causes:**
1. Whitespace in middle of string (intentional behavior)
2. Non-standard whitespace characters

**Solutions:**
```cpp
String text = "hello   world";
String trimmed = trim(text);
// Result: "hello   world" (internal spaces preserved)
// This is correct behavior!

// To remove all whitespace:
String removeAllSpaces(String str) {
    str.replace(" ", "");
    str.replace("\t", "");
    str.replace("\n", "");
    str.replace("\r", "");
    return str;
}
```

## Testing

The library includes comprehensive unit tests. To run:

```bash
pio test -f test_string_utils
```

**Test Coverage:**
- UTF-8 character counting (ASCII, multi-byte, mixed)
- Byte length validation
- Split with various delimiters and configurations
- Trim with different whitespace combinations
- Edge cases (empty strings, all whitespace, no delimiters)
- Memory safety and performance benchmarks

## Example: Complete Application

```cpp
#include <Arduino.h>
#include "string_utils.hpp"

// Sensor data logger with CSV export

struct SensorReading {
    unsigned long timestamp;
    float temperature;
    int humidity;
};

std::vector<SensorReading> readings;

void logReading(float temp, int humidity) {
    SensorReading reading;
    reading.timestamp = millis();
    reading.temperature = temp;
    reading.humidity = humidity;
    readings.push_back(reading);
}

String exportCSV() {
    String csv = "Timestamp,Temperature,Humidity\n";
    
    for (const auto& reading : readings) {
        csv += String(reading.timestamp) + ",";
        csv += String(reading.temperature, 1) + ",";
        csv += String(reading.humidity) + "\n";
    }
    
    return csv;
}

void importCSV(String csvData) {
    readings.clear();
    
    std::vector<String> lines = split(csvData, '\n');
    
    // Skip header
    for (size_t i = 1; i < lines.size(); i++) {
        String line = trim(lines[i]);
        if (line.length() == 0) continue;
        
        std::vector<String> fields = split(line, ',');
        if (fields.size() >= 3) {
            SensorReading reading;
            reading.timestamp = trim(fields[0]).toInt();
            reading.temperature = trim(fields[1]).toFloat();
            reading.humidity = trim(fields[2]).toInt();
            readings.push_back(reading);
        }
    }
}

void handleCommand(String cmd) {
    cmd = trim(cmd);
    std::vector<String> parts = split(cmd, ' ');
    
    if (parts.empty()) return;
    
    String command = parts[0];
    command.toUpperCase();
    
    if (command == "LOG" && parts.size() >= 3) {
        float temp = trim(parts[1]).toFloat();
        int humidity = trim(parts[2]).toInt();
        logReading(temp, humidity);
        Serial.println("OK: Reading logged");
        
    } else if (command == "EXPORT") {
        String csv = exportCSV();
        Serial.println(csv);
        
    } else if (command == "COUNT") {
        Serial.printf("OK: %d readings\n", readings.size());
        
    } else if (command == "CLEAR") {
        readings.clear();
        Serial.println("OK: Cleared");
        
    } else {
        Serial.println("ERROR: Unknown command");
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("Sensor Logger Ready");
    Serial.println("Commands: LOG temp humidity | EXPORT | COUNT | CLEAR");
}

void loop() {
    if (Serial.available()) {
        String input = Serial.readStringUntil('\n');
        handleCommand(input);
    }
    
    // Simulate sensor readings every 10 seconds
    static unsigned long lastReading = 0;
    if (millis() - lastReading > 10000) {
        lastReading = millis();
        float temp = 20.0 + random(-50, 100) / 10.0;
        int humidity = 40 + random(0, 40);
        logReading(temp, humidity);
    }
}
```

## Platform-Specific Notes

### ESP32
- Heap is generally sufficient for typical string operations
- Watch for fragmentation with many split operations
- Use PSRAM for very large datasets if available

### ESP8266
- More memory-constrained than ESP32
- Limit split operations on large strings
- Process data incrementally when possible

### Arduino (AVR)
- Very limited RAM (2KB on Uno)
- Use split sparingly
- Consider streaming approaches for large data

## Performance Tips

1. **Pre-reserve vectors** when you know the approximate size:
   ```cpp
   std::vector<String> tokens;
   tokens.reserve(10);  // If you expect ~10 tokens
   ```

2. **Use references** to avoid copies:
   ```cpp
   for (const String& token : tokens) {  // Reference, not copy
       processToken(token);
   }
   ```

3. **Clear when done** with large datasets:
   ```cpp
   tokens.clear();
   tokens.shrink_to_fit();  // Free memory
   ```

4. **Avoid repeated splits** on the same data:
   ```cpp
   // Cache the result if needed multiple times
   auto tokens = split(data, ',');
   ```

5. **Use `std::string`** for performance-critical code (faster than Arduino String in most cases).

## License

This component is subject to the main project license (see `LICENSE`).

---

**Need help?** Review the examples above or examine the test suite for detailed usage scenarios.

````