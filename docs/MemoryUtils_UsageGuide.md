# Memory Utilities - Usage Guide

## Quick Start

```cpp
#include "memory_utils.hpp"

void setup() {
    Serial.begin(115200);
    
    // Check available memory
    size_t free = freeRam();
    Serial.printf("Free memory: %u bytes\n", free);
    
    // Monitor memory usage (ESP32 only)
    #if defined(ARDUINO_ARCH_ESP32)
    float usage = heapUsagePercent();
    Serial.printf("Heap usage: %.1f%%\n", usage);
    #endif
}
```

## Overview

The Memory Utilities library provides a lightweight, header-only collection of functions for monitoring and managing heap memory on embedded systems. It offers platform-specific implementations optimized for ESP32 while maintaining compatibility with other Arduino-compatible platforms.

**Key Features:**
- Real-time heap memory monitoring
- Heap fragmentation detection (ESP32)
- Memory usage statistics and trends (ESP32)
- Zero runtime overhead (inline functions)
- Header-only implementation (no linking required)
- Platform-aware with graceful fallbacks
- `noexcept` guarantees for safety-critical code

## Platform Support

### ESP32 (Full Support)
All functions available with accurate ESP-IDF heap APIs:
- `freeRam()` - Current free heap
- `minFreeRam()` - Minimum free heap since boot (low-water mark)
- `largestFreeBlock()` - Largest contiguous free block
- `totalHeap()` - Total heap size
- `heapUsagePercent()` - Heap usage percentage

### Other Platforms (Basic Support)
Limited functionality using stack-based estimation:
- `freeRam()` - Approximate free RAM (stack-based)
- Other functions return fallback values (0 or current free RAM)

## Namespace

All functions are in the `HubMemoryUtils` namespace, with convenience `using` declarations for unqualified access.

```cpp
// Option 1: Use namespace (recommended for clarity)
#include "memory_utils.hpp"
size_t free = HubMemoryUtils::freeRam();

// Option 2: Unqualified (convenience using declarations included)
#include "memory_utils.hpp"
size_t free = freeRam();  // Works due to using declaration
```

## API Reference

### `freeRam()` - Get Free Heap Memory

```cpp
size_t freeRam() noexcept;
```

Returns the amount of free heap memory currently available.

**Platform Behavior:**
- **ESP32**: Uses `esp_get_free_heap_size()` for accurate heap measurement across all memory regions
- **Other platforms**: Estimates free RAM using stack pointer technique (approximation)

**Returns:** Free heap memory in bytes, or 0 if unable to determine

**Performance:** O(1) - Fast, safe to call frequently

**Example:**

```cpp
void setup() {
    Serial.begin(115200);
    
    size_t free = freeRam();
    Serial.printf("Available memory: %u bytes\n", free);
    
    // Check before large allocation
    if (free > 10000) {
        uint8_t* buffer = new uint8_t[8192];
        // Use buffer...
        delete[] buffer;
    } else {
        Serial.println("Insufficient memory!");
    }
}
```

**Use Cases:**
- Pre-allocation memory checks
- Memory leak detection
- System health monitoring
- Dynamic resource management
- Diagnostics and logging

**Notes:**
- On ESP32, includes all heap regions (DRAM, SPIRAM if enabled)
- For internal-only memory on ESP32, use `esp_get_free_internal_heap_size()` directly
- On non-ESP32, value is approximate based on stack position
- Returns `size_t` (unsigned) - always check before subtraction to avoid underflow

### `minFreeRam()` - Get Minimum Free Heap (Low-Water Mark)

```cpp
size_t minFreeRam() noexcept;
```

Returns the minimum amount of free heap memory recorded since boot. This is the "low-water mark" that indicates peak memory usage.

**Platform Behavior:**
- **ESP32**: Uses `esp_get_minimum_free_heap_size()` for accurate tracking
- **Other platforms**: Returns current `freeRam()` value (no historical tracking)

**Returns:** Minimum free heap size in bytes since boot

**Performance:** O(1) - Very fast, tracked automatically by ESP-IDF

**Example:**

```cpp
void loop() {
    static unsigned long lastCheck = 0;
    
    if (millis() - lastCheck > 60000) {  // Check every minute
        lastCheck = millis();
        
        size_t current = freeRam();
        size_t minimum = minFreeRam();
        size_t peak_usage = current - minimum;
        
        Serial.printf("Current free: %u bytes\n", current);
        Serial.printf("Minimum free: %u bytes (peak used %u more)\n", 
                      minimum, peak_usage);
        
        // Alert if getting too low
        if (minimum < 10000) {
            Serial.println("WARNING: Memory critically low!");
        }
    }
}
```

**Use Cases:**
- Detecting memory usage peaks
- Ensuring adequate memory headroom
- Memory leak detection over time
- Capacity planning for production
- Stress testing validation

**Notes:**
- ESP32 only - other platforms return current free RAM
- Tracked automatically by ESP-IDF (no performance cost)
- Never resets until reboot
- Ideal for finding worst-case memory scenarios

### `largestFreeBlock()` - Get Largest Contiguous Block

```cpp
size_t largestFreeBlock() noexcept;
```

Returns the size of the largest contiguous free block in the heap. Critical for determining if large allocations will succeed despite total free memory.

**Platform Behavior:**
- **ESP32**: Uses `heap_caps_get_largest_free_block()` for accurate fragmentation analysis
- **Other platforms**: Returns current `freeRam()` value (no fragmentation detection)

**Returns:** Size of largest free heap block in bytes

**Performance:** O(n) where n is number of free blocks - may be slower than `freeRam()`

**Example:**

```cpp
void allocateLargeBuffer() {
    const size_t needed = 16384;  // 16KB buffer
    
    size_t total_free = freeRam();
    size_t largest_block = largestFreeBlock();
    
    Serial.printf("Total free: %u bytes\n", total_free);
    Serial.printf("Largest block: %u bytes\n", largest_block);
    
    if (largest_block >= needed) {
        uint8_t* buffer = new uint8_t[needed];
        if (buffer) {
            Serial.println("Allocation successful");
            // Use buffer...
            delete[] buffer;
        }
    } else {
        Serial.printf("Cannot allocate %u bytes - heap fragmented!\n", needed);
        Serial.printf("Total free: %u, but largest block only %u\n", 
                      total_free, largest_block);
    }
}
```

**Use Cases:**
- Pre-checking large allocations
- Heap fragmentation detection
- Deciding between allocation strategies
- Triggering heap compaction/reorganization
- Dynamic buffer sizing

**Notes:**
- **ESP32 only** - crucial for fragmentation awareness
- Can be significantly smaller than `freeRam()` due to fragmentation
- May have slight performance cost (scans free blocks)
- Call before attempting large allocations

**Understanding Fragmentation:**

```cpp
// Example fragmentation scenario:
// Total free: 20KB
// Largest block: 4KB
// Heap layout: [used][4KB free][used][3KB free][used][2KB free]...
// Cannot allocate 8KB despite having 20KB total free!

void checkFragmentation() {
    size_t total = freeRam();
    size_t largest = largestFreeBlock();
    
    float fragmentation = 100.0 * (1.0 - (float)largest / (float)total);
    Serial.printf("Fragmentation: %.1f%%\n", fragmentation);
    
    if (fragmentation > 50.0) {
        Serial.println("Heap is highly fragmented!");
    }
}
```

### `totalHeap()` - Get Total Heap Size

```cpp
size_t totalHeap() noexcept;
```

Returns the total size of the heap available to the application.

**Platform Behavior:**
- **ESP32**: Uses `heap_caps_get_total_size()` for accurate total heap size
- **Other platforms**: Returns 0 (not available)

**Returns:** Total heap size in bytes, or 0 on non-ESP32 platforms

**Performance:** O(1) - Very fast, value is fixed at boot

**Example:**

```cpp
void printMemoryInfo() {
    #if defined(ARDUINO_ARCH_ESP32)
    size_t total = totalHeap();
    size_t free = freeRam();
    size_t used = total - free;
    
    Serial.printf("Total heap: %u bytes (%.1f KB)\n", total, total / 1024.0);
    Serial.printf("Used: %u bytes (%.1f KB)\n", used, used / 1024.0);
    Serial.printf("Free: %u bytes (%.1f KB)\n", free, free / 1024.0);
    Serial.printf("Usage: %.1f%%\n", 100.0 * used / total);
    #else
    Serial.println("Memory statistics not available on this platform");
    #endif
}
```

**Use Cases:**
- System information display
- Calculating usage percentages
- Capacity planning
- Comparing different hardware configurations
- Boot-time diagnostics

**Notes:**
- **ESP32 only** - returns 0 on other platforms
- Value is fixed at boot time
- Includes all heap regions (varies by board and PSRAM config)
- Typical values: 320KB (ESP32 WROOM), 4MB+ (ESP32 with PSRAM)

### `heapUsagePercent()` - Get Heap Usage Percentage

```cpp
float heapUsagePercent() noexcept;
```

Convenience function that calculates current heap usage as a percentage.

**Platform Behavior:**
- **ESP32**: Calculates from `totalHeap()` and `freeRam()`
- **Other platforms**: Returns 0.0 (not available)

**Returns:** Heap usage percentage (0.0 to 100.0), or 0.0 on non-ESP32

**Performance:** O(1) - Very fast

**Example:**

```cpp
void monitorMemory() {
    #if defined(ARDUINO_ARCH_ESP32)
    float usage = heapUsagePercent();
    
    if (usage > 90.0) {
        Serial.println("CRITICAL: Memory usage over 90%!");
        // Take corrective action...
    } else if (usage > 75.0) {
        Serial.println("WARNING: Memory usage over 75%");
    } else {
        Serial.printf("Memory usage: %.1f%% (OK)\n", usage);
    }
    #endif
}

void loop() {
    static unsigned long lastCheck = 0;
    
    if (millis() - lastCheck > 5000) {  // Every 5 seconds
        lastCheck = millis();
        monitorMemory();
    }
    
    // Your application code...
}
```

**Use Cases:**
- Memory threshold monitoring
- Triggering cleanup routines
- Load balancing decisions
- User-facing memory indicators
- Automated alerts and logging

**Notes:**
- **ESP32 only** - returns 0.0 on other platforms
- Returns `float` for fractional precision
- Range is 0.0 (empty) to 100.0 (full)
- Convenience wrapper - same as manually calculating percentage

## Common Usage Patterns

### Pattern 1: Memory Leak Detection

```cpp
class MemoryMonitor {
private:
    size_t baseline_free;
    size_t baseline_min;
    unsigned long last_check;
    
public:
    void begin() {
        baseline_free = freeRam();
        #if defined(ARDUINO_ARCH_ESP32)
        baseline_min = minFreeRam();
        #endif
        last_check = millis();
    }
    
    void check() {
        if (millis() - last_check < 30000) return;  // Every 30 seconds
        last_check = millis();
        
        size_t current_free = freeRam();
        int32_t change = (int32_t)current_free - (int32_t)baseline_free;
        
        Serial.printf("Memory: %u bytes (", current_free);
        if (change > 0) {
            Serial.printf("+%d", change);
        } else {
            Serial.printf("%d", change);
        }
        Serial.println(" from baseline)");
        
        #if defined(ARDUINO_ARCH_ESP32)
        size_t current_min = minFreeRam();
        if (current_min < baseline_min) {
            Serial.printf("WARNING: Min free decreased from %u to %u\n", 
                         baseline_min, current_min);
            baseline_min = current_min;  // Track new low
        }
        #endif
        
        // Alert on significant memory loss
        if (change < -10000) {  // Lost more than 10KB
            Serial.println("ALERT: Possible memory leak detected!");
        }
    }
};

MemoryMonitor memMon;

void setup() {
    Serial.begin(115200);
    memMon.begin();
}

void loop() {
    memMon.check();
    // Your application code...
}
```

### Pattern 2: Safe Large Allocation

```cpp
template<typename T>
T* safeAllocate(size_t count, size_t safety_margin = 8192) {
    size_t bytes_needed = count * sizeof(T);
    
    #if defined(ARDUINO_ARCH_ESP32)
    // On ESP32, check both total free and largest block
    size_t free = freeRam();
    size_t largest = largestFreeBlock();
    
    if (largest < bytes_needed) {
        Serial.printf("Allocation failed: Need %u bytes, but largest block is %u\n",
                     bytes_needed, largest);
        return nullptr;
    }
    
    if (free < bytes_needed + safety_margin) {
        Serial.printf("Allocation refused: Would leave only %d bytes free\n",
                     (int)(free - bytes_needed));
        return nullptr;
    }
    #else
    // On other platforms, simple check
    size_t free = freeRam();
    if (free < bytes_needed + safety_margin) {
        Serial.println("Insufficient memory for allocation");
        return nullptr;
    }
    #endif
    
    T* ptr = new (std::nothrow) T[count];
    
    if (ptr) {
        Serial.printf("Allocated %u bytes successfully\n", bytes_needed);
    } else {
        Serial.println("Allocation failed despite checks!");
    }
    
    return ptr;
}

void example() {
    uint8_t* buffer = safeAllocate<uint8_t>(16384);  // 16KB
    if (buffer) {
        // Use buffer...
        delete[] buffer;
    }
}
```

### Pattern 3: Dynamic Resource Scaling

```cpp
class AdaptiveBuffer {
private:
    uint8_t* buffer;
    size_t buffer_size;
    
public:
    AdaptiveBuffer() : buffer(nullptr), buffer_size(0) {}
    
    ~AdaptiveBuffer() {
        free();
    }
    
    bool allocate() {
        // Determine buffer size based on available memory
        #if defined(ARDUINO_ARCH_ESP32)
        size_t available = largestFreeBlock();
        #else
        size_t available = freeRam();
        #endif
        
        // Use up to 50% of available memory, with limits
        size_t target_size = available / 2;
        target_size = constrain(target_size, 1024, 65536);  // 1KB to 64KB
        
        buffer = new (std::nothrow) uint8_t[target_size];
        if (buffer) {
            buffer_size = target_size;
            Serial.printf("Allocated %u byte buffer\n", buffer_size);
            return true;
        }
        
        Serial.println("Buffer allocation failed");
        return false;
    }
    
    void free() {
        if (buffer) {
            delete[] buffer;
            buffer = nullptr;
            buffer_size = 0;
        }
    }
    
    size_t size() const { return buffer_size; }
    uint8_t* data() { return buffer; }
};

void example() {
    AdaptiveBuffer buf;
    if (buf.allocate()) {
        // Use buffer sized appropriately for available memory
        Serial.printf("Working with %u byte buffer\n", buf.size());
    }
}
```

### Pattern 4: Memory Threshold Actions

```cpp
class MemoryManager {
private:
    enum MemoryState {
        ABUNDANT,  // > 75% free
        NORMAL,    // 50-75% free
        LOW,       // 25-50% free
        CRITICAL   // < 25% free
    };
    
    MemoryState state;
    
public:
    MemoryManager() : state(ABUNDANT) {}
    
    void update() {
        #if defined(ARDUINO_ARCH_ESP32)
        float usage = heapUsagePercent();
        MemoryState new_state;
        
        if (usage < 25.0) {
            new_state = ABUNDANT;
        } else if (usage < 50.0) {
            new_state = NORMAL;
        } else if (usage < 75.0) {
            new_state = LOW;
        } else {
            new_state = CRITICAL;
        }
        
        if (new_state != state) {
            state = new_state;
            onStateChange(state);
        }
        #endif
    }
    
    void onStateChange(MemoryState new_state) {
        switch (new_state) {
            case ABUNDANT:
                Serial.println("Memory: ABUNDANT");
                enableAllFeatures();
                break;
                
            case NORMAL:
                Serial.println("Memory: NORMAL");
                // No action needed
                break;
                
            case LOW:
                Serial.println("Memory: LOW - reducing features");
                disableNonEssentialFeatures();
                runGarbageCollection();
                break;
                
            case CRITICAL:
                Serial.println("Memory: CRITICAL - emergency measures");
                disableAllNonEssentialFeatures();
                releaseAllCaches();
                runAggressiveGarbageCollection();
                break;
        }
    }
    
    void enableAllFeatures() {
        // Re-enable optional features
    }
    
    void disableNonEssentialFeatures() {
        // Disable logging, caching, etc.
    }
    
    void disableAllNonEssentialFeatures() {
        // Minimal operation mode
    }
    
    void releaseAllCaches() {
        // Clear all cached data
    }
    
    void runGarbageCollection() {
        // Trigger cleanup routines
    }
    
    void runAggressiveGarbageCollection() {
        // More thorough cleanup
    }
};

MemoryManager memMgr;

void loop() {
    static unsigned long lastCheck = 0;
    
    if (millis() - lastCheck > 1000) {  // Check every second
        lastCheck = millis();
        memMgr.update();
    }
    
    // Application code...
}
```

### Pattern 5: Memory Statistics Dashboard

```cpp
void printMemoryDashboard() {
    Serial.println("=== Memory Dashboard ===");
    
    size_t free = freeRam();
    Serial.printf("Free RAM: %u bytes (%.2f KB)\n", free, free / 1024.0);
    
    #if defined(ARDUINO_ARCH_ESP32)
    size_t total = totalHeap();
    size_t used = total - free;
    size_t minimum = minFreeRam();
    size_t largest = largestFreeBlock();
    float usage_pct = heapUsagePercent();
    
    Serial.printf("Total Heap: %u bytes (%.2f KB)\n", total, total / 1024.0);
    Serial.printf("Used: %u bytes (%.2f KB)\n", used, used / 1024.0);
    Serial.printf("Usage: %.1f%%\n", usage_pct);
    Serial.println();
    
    Serial.printf("Minimum Free (ever): %u bytes (%.2f KB)\n", 
                  minimum, minimum / 1024.0);
    Serial.printf("Peak Usage: %u bytes (%.2f KB)\n", 
                  total - minimum, (total - minimum) / 1024.0);
    Serial.println();
    
    Serial.printf("Largest Free Block: %u bytes (%.2f KB)\n", 
                  largest, largest / 1024.0);
    
    // Calculate fragmentation
    float frag_pct = 100.0 * (1.0 - (float)largest / (float)free);
    Serial.printf("Fragmentation: %.1f%%\n", frag_pct);
    
    // Memory health assessment
    Serial.print("Status: ");
    if (usage_pct > 90.0) {
        Serial.println("CRITICAL");
    } else if (usage_pct > 75.0) {
        Serial.println("WARNING");
    } else if (frag_pct > 50.0) {
        Serial.println("FRAGMENTED");
    } else {
        Serial.println("HEALTHY");
    }
    #else
    Serial.println("(Detailed stats available on ESP32 only)");
    #endif
    
    Serial.println("=======================");
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    printMemoryDashboard();
}
```

### Pattern 6: Periodic Memory Logging

```cpp
class MemoryLogger {
private:
    struct MemorySnapshot {
        unsigned long timestamp;
        size_t free;
        #if defined(ARDUINO_ARCH_ESP32)
        size_t largest_block;
        float usage_percent;
        #endif
    };
    
    static const size_t MAX_SNAPSHOTS = 100;
    MemorySnapshot snapshots[MAX_SNAPSHOTS];
    size_t snapshot_count;
    size_t next_index;
    
public:
    MemoryLogger() : snapshot_count(0), next_index(0) {}
    
    void recordSnapshot() {
        MemorySnapshot snap;
        snap.timestamp = millis();
        snap.free = freeRam();
        
        #if defined(ARDUINO_ARCH_ESP32)
        snap.largest_block = largestFreeBlock();
        snap.usage_percent = heapUsagePercent();
        #endif
        
        snapshots[next_index] = snap;
        next_index = (next_index + 1) % MAX_SNAPSHOTS;
        
        if (snapshot_count < MAX_SNAPSHOTS) {
            snapshot_count++;
        }
    }
    
    void printHistory() {
        Serial.printf("=== Memory History (%u snapshots) ===\n", snapshot_count);
        
        size_t start = (snapshot_count < MAX_SNAPSHOTS) ? 0 : next_index;
        
        for (size_t i = 0; i < snapshot_count; i++) {
            size_t idx = (start + i) % MAX_SNAPSHOTS;
            const MemorySnapshot& snap = snapshots[idx];
            
            Serial.printf("[%lu] Free: %u", snap.timestamp, snap.free);
            
            #if defined(ARDUINO_ARCH_ESP32)
            Serial.printf(", Largest: %u, Usage: %.1f%%", 
                         snap.largest_block, snap.usage_percent);
            #endif
            
            Serial.println();
        }
        
        Serial.println("================================");
    }
    
    void analyzetrends() {
        if (snapshot_count < 2) {
            Serial.println("Insufficient data for trend analysis");
            return;
        }
        
        // Calculate trend (simple linear approximation)
        int32_t first_free = snapshots[0].free;
        int32_t last_free = snapshots[snapshot_count - 1].free;
        int32_t change = last_free - first_free;
        
        Serial.printf("Memory trend: %d bytes ", change);
        if (change < -1000) {
            Serial.println("(DECREASING - possible leak)");
        } else if (change > 1000) {
            Serial.println("(INCREASING - good)");
        } else {
            Serial.println("(STABLE)");
        }
    }
};

MemoryLogger logger;

void loop() {
    static unsigned long lastLog = 0;
    
    if (millis() - lastLog > 10000) {  // Every 10 seconds
        lastLog = millis();
        logger.recordSnapshot();
    }
    
    // Your application code...
}
```

### Pattern 7: Pre-Flight Memory Check

```cpp
bool preFlightMemoryCheck() {
    Serial.println("=== Pre-Flight Memory Check ===");
    
    size_t free = freeRam();
    Serial.printf("Free RAM: %u bytes\n", free);
    
    // Minimum required: 50KB
    const size_t MIN_REQUIRED = 50000;
    if (free < MIN_REQUIRED) {
        Serial.printf("FAIL: Insufficient memory (need %u bytes)\n", MIN_REQUIRED);
        return false;
    }
    
    #if defined(ARDUINO_ARCH_ESP32)
    // Check fragmentation
    size_t largest = largestFreeBlock();
    Serial.printf("Largest block: %u bytes\n", largest);
    
    const size_t MIN_BLOCK = 32768;  // Need 32KB contiguous
    if (largest < MIN_BLOCK) {
        Serial.printf("FAIL: Heap too fragmented (need %u byte block)\n", MIN_BLOCK);
        return false;
    }
    
    // Check usage
    float usage = heapUsagePercent();
    Serial.printf("Usage: %.1f%%\n", usage);
    
    if (usage > 80.0) {
        Serial.println("WARN: Memory usage high");
    }
    
    // Check historical minimum
    size_t minimum = minFreeRam();
    Serial.printf("Historical minimum: %u bytes\n", minimum);
    
    if (minimum < MIN_REQUIRED) {
        Serial.println("WARN: Memory has been critically low");
    }
    #endif
    
    Serial.println("PASS: Memory check successful");
    return true;
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    if (!preFlightMemoryCheck()) {
        Serial.println("Cannot start application - insufficient memory");
        while (1) {
            delay(1000);
        }
    }
    
    // Continue with application setup...
}
```

## Memory Considerations

### ESP32 Memory Architecture

The ESP32 has multiple memory regions:

- **DRAM (Data RAM)**: ~160-320 KB depending on model
- **IRAM (Instruction RAM)**: Shared with DRAM
- **SPIRAM (External PSRAM)**: Optional, 4-8 MB (if equipped)

The `freeRam()` function on ESP32 includes all available heap regions by default.

### Memory Allocation Best Practices

✅ **DO:**
- Check available memory before large allocations
- Monitor `minFreeRam()` to ensure adequate headroom (recommend 20-30% free)
- Use `largestFreeBlock()` before allocating large contiguous buffers
- Free resources promptly when no longer needed
- Use stack allocation for small, temporary data
- Consider memory pools for frequent allocations/deallocations

❌ **DON'T:**
- Allocate without checking availability
- Ignore heap fragmentation on ESP32
- Leak memory (always match `new` with `delete`)
- Allocate in interrupt handlers
- Create many small allocations (causes fragmentation)
- Assume infinite memory is available

### Fragmentation Management

Heap fragmentation occurs when free memory is split into many small non-contiguous blocks:

```cpp
void demonstrateFragmentation() {
    // Allocate several buffers
    uint8_t* buffers[10];
    for (int i = 0; i < 10; i++) {
        buffers[i] = new uint8_t[4096];  // 4KB each
    }
    
    // Free every other buffer
    for (int i = 0; i < 10; i += 2) {
        delete[] buffers[i];
        buffers[i] = nullptr;
    }
    
    // Now heap is fragmented:
    // [used][4KB free][used][4KB free][used]...
    
    size_t free = freeRam();        // May show 20KB free
    size_t largest = largestFreeBlock();  // Only 4KB blocks available
    
    Serial.printf("Free: %u, Largest: %u\n", free, largest);
    // Cannot allocate 8KB despite 20KB being free!
    
    // Cleanup
    for (int i = 1; i < 10; i += 2) {
        delete[] buffers[i];
    }
}
```

**Strategies to reduce fragmentation:**
1. Use memory pools for fixed-size allocations
2. Allocate large buffers early and keep them
3. Avoid frequent allocation/deallocation cycles
4. Use static or stack allocation when possible
5. Consider pre-allocating worst-case buffers

## Performance Characteristics

| Function | ESP32 | Other Platforms | Notes |
|----------|-------|----------------|-------|
| `freeRam()` | O(1), ~1 µs | O(1), ~5 µs | Very fast |
| `minFreeRam()` | O(1), ~1 µs | O(1), ~5 µs | Cached value |
| `largestFreeBlock()` | O(n), ~50 µs | O(1), ~5 µs | Scans heap blocks |
| `totalHeap()` | O(1), ~1 µs | O(1), instant | Constant |
| `heapUsagePercent()` | O(1), ~2 µs | O(1), instant | Simple calculation |

**Note:** Times are approximate for ESP32 @ 240MHz. The `largestFreeBlock()` function is the slowest as it must scan the heap's free list.

### Call Frequency Recommendations

- `freeRam()`: Safe to call frequently (every loop iteration if needed)
- `minFreeRam()`: Safe to call frequently (every loop iteration)
- `largestFreeBlock()`: Call sparingly (before large allocations, periodic monitoring)
- `totalHeap()`: Can be called anytime (constant value)
- `heapUsagePercent()`: Safe to call frequently (every few seconds for monitoring)

## Platform-Specific Notes

### ESP32
- Full feature support with accurate ESP-IDF APIs
- Includes SPIRAM in total if configured
- Excellent fragmentation detection via `largestFreeBlock()`
- Hardware-accelerated memory management
- Typical heap: 200-320 KB internal, up to 4-8 MB with PSRAM

### ESP8266
- Basic support using stack-based estimation
- More memory constrained (~40-80 KB)
- No built-in fragmentation detection
- Watch memory carefully on this platform

### Arduino (AVR - Uno, Mega, etc.)
- Very limited RAM (2 KB Uno, 8 KB Mega)
- Stack-based estimation only
- Every byte counts on these platforms
- Avoid dynamic allocation when possible

### Arduino Due, Teensy, others
- Stack-based estimation
- Usually more RAM than AVR
- No advanced heap statistics

## Troubleshooting

### Issue: Allocation fails despite sufficient free memory

**Symptoms:** `new` returns `nullptr` even though `freeRam()` shows enough memory

**Causes:**
1. Heap fragmentation (ESP32)
2. Requested block larger than any contiguous free space

**Solutions:**
```cpp
#if defined(ARDUINO_ARCH_ESP32)
size_t largest = largestFreeBlock();
if (largest < needed_size) {
    Serial.println("Heap is too fragmented for this allocation");
    // Consider:
    // 1. Freeing other allocations first
    // 2. Reducing allocation size
    // 3. Using a memory pool
    // 4. Restructuring to avoid large contiguous needs
}
#endif
```

### Issue: Memory constantly decreasing

**Symptoms:** `freeRam()` steadily decreases over time, `minFreeRam()` keeps dropping

**Causes:**
1. Memory leak (missing `delete`)
2. Growing data structures without bounds
3. String concatenation in loops

**Solutions:**
```cpp
// Track memory over time
void detectLeak() {
    static size_t last_min = 0;
    size_t current_min = minFreeRam();
    
    if (last_min != 0 && current_min < last_min) {
        Serial.printf("WARNING: Minimum free dropped from %u to %u\n",
                     last_min, current_min);
    }
    
    last_min = current_min;
}

// Check for leaks:
// 1. Review all new/delete pairs
// 2. Check for growing vectors/strings
// 3. Use valgrind or similar tools (desktop simulation)
// 4. Add memory tracking to allocations
```

### Issue: Memory readings seem inaccurate (non-ESP32)

**Symptoms:** `freeRam()` returns unexpected values on Arduino/AVR platforms

**Causes:**
1. Stack-based estimation is approximate
2. Stack grows downward, varies with call depth
3. Different optimization levels affect stack usage

**Solutions:**
- Accept that values are approximate on non-ESP32
- Use relative changes rather than absolute values
- Consider upgrading to ESP32 for accurate monitoring
- Test in real-world conditions, not just at boot

### Issue: `largestFreeBlock()` much smaller than `freeRam()`

**Symptoms:** Large gap between total free and largest block

**Cause:** Normal heap fragmentation

**Solutions:**
```cpp
void handleFragmentation() {
    size_t free = freeRam();
    size_t largest = largestFreeBlock();
    float frag = 100.0 * (1.0 - (float)largest / (float)free);
    
    if (frag > 50.0) {
        Serial.printf("Fragmentation: %.1f%% - Consider:\n", frag);
        Serial.println("1. Using smaller allocations");
        Serial.println("2. Using memory pools");
        Serial.println("3. Reducing allocation churn");
        Serial.println("4. Restarting if possible");
    }
}
```

## Integration with Other Libraries

### With Serial Logging

```cpp
#include "memory_utils.hpp"

void logMemoryState(const char* label) {
    Serial.printf("[%s] Free: %u bytes", label, freeRam());
    
    #if defined(ARDUINO_ARCH_ESP32)
    Serial.printf(", Usage: %.1f%%", heapUsagePercent());
    #endif
    
    Serial.println();
}

void setup() {
    Serial.begin(115200);
    logMemoryState("Boot");
}

void loop() {
    logMemoryState("Loop");
    delay(5000);
}
```

### With WiFi Connection

```cpp
#include "memory_utils.hpp"
#include <WiFi.h>

void connectWiFi(const char* ssid, const char* password) {
    // Check memory before WiFi init (uses significant RAM)
    size_t free_before = freeRam();
    Serial.printf("Memory before WiFi: %u bytes\n", free_before);
    
    WiFi.begin(ssid, password);
    
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    
    size_t free_after = freeRam();
    Serial.printf("\nMemory after WiFi: %u bytes (used %d)\n", 
                  free_after, (int)(free_before - free_after));
}
```

### With Web Server

```cpp
#include "memory_utils.hpp"
#include <WebServer.h>

WebServer server(80);

void handleMemoryStatus() {
    String json = "{";
    json += "\"free\":" + String(freeRam());
    
    #if defined(ARDUINO_ARCH_ESP32)
    json += ",\"total\":" + String(totalHeap());
    json += ",\"usage\":" + String(heapUsagePercent());
    json += ",\"minimum\":" + String(minFreeRam());
    json += ",\"largest_block\":" + String(largestFreeBlock());
    #endif
    
    json += "}";
    
    server.send(200, "application/json", json);
}

void setup() {
    WiFi.begin("ssid", "password");
    while (WiFi.status() != WL_CONNECTED) delay(500);
    
    server.on("/api/memory", handleMemoryStatus);
    server.begin();
}

void loop() {
    server.handleClient();
}
```

### With MQTT Telemetry

```cpp
#include "memory_utils.hpp"
#include <PubSubClient.h>

WiFiClient espClient;
PubSubClient mqtt(espClient);

void publishMemoryMetrics() {
    if (!mqtt.connected()) return;
    
    char topic[64];
    char payload[32];
    
    snprintf(topic, sizeof(topic), "device/%s/memory/free", deviceId);
    snprintf(payload, sizeof(payload), "%u", freeRam());
    mqtt.publish(topic, payload);
    
    #if defined(ARDUINO_ARCH_ESP32)
    snprintf(topic, sizeof(topic), "device/%s/memory/usage", deviceId);
    snprintf(payload, sizeof(payload), "%.1f", heapUsagePercent());
    mqtt.publish(topic, payload);
    
    snprintf(topic, sizeof(topic), "device/%s/memory/minimum", deviceId);
    snprintf(payload, sizeof(payload), "%u", minFreeRam());
    mqtt.publish(topic, payload);
    #endif
}

void loop() {
    static unsigned long lastPublish = 0;
    
    if (millis() - lastPublish > 60000) {  // Every minute
        lastPublish = millis();
        publishMemoryMetrics();
    }
    
    mqtt.loop();
}
```

## Testing

To verify memory utilities in your project:

```cpp
void testMemoryUtils() {
    Serial.println("=== Memory Utils Test ===");
    
    // Test 1: Basic free RAM reading
    size_t free1 = freeRam();
    Serial.printf("Test 1 - Free RAM: %u bytes ", free1);
    Serial.println(free1 > 0 ? "[PASS]" : "[FAIL]");
    
    // Test 2: Allocation reduces free memory
    const size_t test_size = 1024;
    uint8_t* test_buffer = new uint8_t[test_size];
    size_t free2 = freeRam();
    bool reduced = free2 < free1;
    Serial.printf("Test 2 - Allocation reduces free: ");
    Serial.println(reduced ? "[PASS]" : "[FAIL]");
    
    // Test 3: Deallocation restores memory
    delete[] test_buffer;
    size_t free3 = freeRam();
    bool restored = (free3 >= free1 - 100);  // Allow small variance
    Serial.printf("Test 3 - Deallocation restores: ");
    Serial.println(restored ? "[PASS]" : "[FAIL]");
    
    #if defined(ARDUINO_ARCH_ESP32)
    // Test 4: ESP32-specific functions
    size_t min_free = minFreeRam();
    size_t total = totalHeap();
    size_t largest = largestFreeBlock();
    float usage = heapUsagePercent();
    
    Serial.printf("Test 4 - ESP32 functions:\n");
    Serial.printf("  Min free: %u ", min_free);
    Serial.println(min_free > 0 ? "[PASS]" : "[FAIL]");
    Serial.printf("  Total: %u ", total);
    Serial.println(total > free1 ? "[PASS]" : "[FAIL]");
    Serial.printf("  Largest block: %u ", largest);
    Serial.println(largest > 0 && largest <= free1 ? "[PASS]" : "[FAIL]");
    Serial.printf("  Usage: %.1f%% ", usage);
    Serial.println(usage >= 0.0 && usage <= 100.0 ? "[PASS]" : "[FAIL]");
    #endif
    
    Serial.println("=========================");
}

void setup() {
    Serial.begin(115200);
    delay(2000);
    testMemoryUtils();
}
```

## Example: Complete Memory Monitoring System

```cpp
#include <Arduino.h>
#include "memory_utils.hpp"

class MemoryMonitoringSystem {
private:
    // Configuration
    static const unsigned long CHECK_INTERVAL_MS = 5000;
    static const size_t CRITICAL_THRESHOLD = 10000;  // 10KB
    static const size_t WARNING_THRESHOLD = 50000;   // 50KB
    
    // State
    unsigned long last_check;
    size_t baseline_free;
    bool alert_sent;
    
    // Statistics
    size_t min_observed;
    size_t max_observed;
    unsigned long total_checks;
    
public:
    MemoryMonitoringSystem() 
        : last_check(0)
        , baseline_free(0)
        , alert_sent(false)
        , min_observed(SIZE_MAX)
        , max_observed(0)
        , total_checks(0)
    {}
    
    void begin() {
        baseline_free = freeRam();
        min_observed = baseline_free;
        max_observed = baseline_free;
        
        Serial.println("=== Memory Monitoring System Started ===");
        Serial.printf("Baseline free memory: %u bytes\n", baseline_free);
        
        #if defined(ARDUINO_ARCH_ESP32)
        Serial.printf("Total heap: %u bytes\n", totalHeap());
        #endif
    }
    
    void update() {
        unsigned long now = millis();
        
        if (now - last_check < CHECK_INTERVAL_MS) {
            return;
        }
        
        last_check = now;
        total_checks++;
        
        size_t current_free = freeRam();
        
        // Update statistics
        if (current_free < min_observed) min_observed = current_free;
        if (current_free > max_observed) max_observed = current_free;
        
        // Check thresholds
        if (current_free < CRITICAL_THRESHOLD) {
            handleCriticalMemory(current_free);
        } else if (current_free < WARNING_THRESHOLD) {
            handleLowMemory(current_free);
            alert_sent = false;  // Reset for next critical
        } else {
            alert_sent = false;  // Reset alert
        }
        
        // Periodic detailed report
        if (total_checks % 12 == 0) {  // Every minute (5s * 12)
            printDetailedReport();
        }
    }
    
    void handleCriticalMemory(size_t free) {
        if (!alert_sent) {
            Serial.println("\n!!! CRITICAL MEMORY ALERT !!!");
            Serial.printf("Free memory: %u bytes (threshold: %u)\n", 
                         free, CRITICAL_THRESHOLD);
            
            #if defined(ARDUINO_ARCH_ESP32)
            Serial.printf("Largest block: %u bytes\n", largestFreeBlock());
            Serial.printf("Usage: %.1f%%\n", heapUsagePercent());
            #endif
            
            // Take emergency action
            performEmergencyCleanup();
            
            alert_sent = true;
        }
    }
    
    void handleLowMemory(size_t free) {
        Serial.printf("WARNING: Low memory - %u bytes free\n", free);
        
        // Take preventive action
        performPreventiveCleanup();
    }
    
    void performEmergencyCleanup() {
        Serial.println("Performing emergency cleanup...");
        // Implement application-specific cleanup
        // e.g., clear caches, reduce buffer sizes, etc.
    }
    
    void performPreventiveCleanup() {
        Serial.println("Performing preventive cleanup...");
        // Implement application-specific preventive measures
    }
    
    void printDetailedReport() {
        Serial.println("\n=== Memory Report ===");
        
        size_t current = freeRam();
        Serial.printf("Current free: %u bytes (%.2f KB)\n", 
                     current, current / 1024.0);
        Serial.printf("Minimum observed: %u bytes (%.2f KB)\n", 
                     min_observed, min_observed / 1024.0);
        Serial.printf("Maximum observed: %u bytes (%.2f KB)\n", 
                     max_observed, max_observed / 1024.0);
        Serial.printf("Range: %u bytes\n", max_observed - min_observed);
        
        int32_t change = (int32_t)current - (int32_t)baseline_free;
        Serial.printf("Change from baseline: %d bytes\n", change);
        
        #if defined(ARDUINO_ARCH_ESP32)
        size_t total = totalHeap();
        size_t minimum = minFreeRam();
        size_t largest = largestFreeBlock();
        float usage = heapUsagePercent();
        
        Serial.printf("\nESP32 Statistics:\n");
        Serial.printf("Total heap: %u bytes (%.2f KB)\n", 
                     total, total / 1024.0);
        Serial.printf("Usage: %.1f%%\n", usage);
        Serial.printf("Min free (ever): %u bytes (%.2f KB)\n", 
                     minimum, minimum / 1024.0);
        Serial.printf("Largest block: %u bytes (%.2f KB)\n", 
                     largest, largest / 1024.0);
        
        float frag = 100.0 * (1.0 - (float)largest / (float)current);
        Serial.printf("Fragmentation: %.1f%%\n", frag);
        #endif
        
        Serial.printf("\nChecks performed: %lu\n", total_checks);
        Serial.println("====================\n");
    }
    
    void printQuickStatus() {
        size_t free = freeRam();
        Serial.printf("Memory: %u bytes", free);
        
        #if defined(ARDUINO_ARCH_ESP32)
        Serial.printf(" (%.1f%% used)", heapUsagePercent());
        #endif
        
        Serial.println();
    }
};

// Global instance
MemoryMonitoringSystem memMonitor;

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("Starting application...");
    memMonitor.begin();
    
    // Your application setup...
}

void loop() {
    // Update memory monitor
    memMonitor.update();
    
    // Your application code...
    
    delay(100);
}
```

## Best Practices Summary

### ✅ DO:

- Check memory before large allocations
- Monitor `minFreeRam()` to track worst-case usage
- Use `largestFreeBlock()` before allocating large contiguous buffers (ESP32)
- Set up periodic memory monitoring in production
- Maintain at least 20-30% free heap as safety margin
- Use `noexcept` guarantee for safety-critical error handling
- Log memory statistics during development
- Test worst-case memory scenarios

### ❌ DON'T:

- Ignore memory constraints on embedded systems
- Allocate without checking availability
- Forget about heap fragmentation (especially on ESP32)
- Let memory leaks accumulate
- Run with critically low memory (<10KB free)
- Assume memory is unlimited
- Ignore the difference between `freeRam()` and `largestFreeBlock()`
- Call `largestFreeBlock()` excessively (has performance cost)

## Performance Tips

1. **Cache values when appropriate:**
   ```cpp
   // DON'T call repeatedly in tight loop
   for (int i = 0; i < 1000; i++) {
       if (freeRam() > 10000) { ... }  // Inefficient
   }
   
   // DO cache the value
   size_t free = freeRam();
   for (int i = 0; i < 1000; i++) {
       if (free > 10000) { ... }  // Efficient
   }
   ```

2. **Use appropriate check intervals:**
   - Critical systems: Every 100ms - 1s
   - Normal monitoring: Every 5-10s
   - Statistics logging: Every 1-5 minutes

3. **Minimize `largestFreeBlock()` calls:**
   ```cpp
   // Only call when needed
   if (need_large_allocation) {
       size_t largest = largestFreeBlock();
       if (largest >= required_size) {
           // Proceed with allocation
       }
   }
   ```

## License

This component is subject to the main project license (see `LICENSE`).

---

**Need help?** Review the examples above or examine the header file for detailed function documentation.
