# ShiftRegister Class - Usage Guide

## Quick Start

```cpp
#include "shift_register.h"

// Create shift register controller for 74HC595 (or similar)
// Connected to pins: Data=5, Clock=6, Latch=7, using 2 cascaded chips
ShiftRegister sr(5, 6, 7, 2);

void setup() {
    // Set individual outputs
    sr.set(0, HIGH);   // Turn on first output
    sr.set(8, HIGH);   // Turn on first output of second chip
    
    // Or set all at once
    sr.setAll(HIGH);   // All outputs ON
}

void loop() {
    // Your application code
}
```

## Overview

The `ShiftRegister` class provides a clean interface for controlling cascaded shift registers (like 74HC595, TPIC6B595, etc.) commonly used to expand digital outputs. This is essential for projects that need more GPIO pins than available on the microcontroller.

**Key Features:**
- Support for 1-8 cascaded shift registers (8-64 outputs)
- MSB-first serial output
- Deferred updates for efficient batch operations
- Memory-safe 64-bit internal state
- Bounds checking on all operations

## Constructor

```cpp
ShiftRegister(uint8_t data_pin, 
              uint8_t clock_pin, 
              uint8_t latch_pin, 
              uint8_t num_registers = 1);
```

### Parameters Explained

- **data_pin**: Serial data pin (DS/SER on 74HC595)
  - Connects to: DS, SER, or SER IN pin
  - Any GPIO pin
  
- **clock_pin**: Shift clock pin (SHCP/SRCLK on 74HC595)
  - Connects to: SHCP, SRCLK, or CLK pin
  - Any GPIO pin
  - Pulses HIGH to shift in each bit
  
- **latch_pin**: Storage register clock/latch pin (STCP/RCLK on 74HC595)
  - Connects to: STCP, RCLK, or LATCH pin
  - Any GPIO pin
  - Pulsed HIGH to transfer shift register contents to output pins
  
- **num_registers**: Number of cascaded shift registers
  - Range: 1-8 (auto-constrained)
  - Default: 1
  - Each register provides 8 additional outputs

### Initialization Behavior

The constructor:
1. Sets all three pins to OUTPUT mode
2. Sets all pins LOW (known state)
3. Initializes internal state to 0 (all outputs off)
4. Constrains `num_registers` to valid range [1, 8]

## Core API

### set() - Set Individual Output

```cpp
bool set(uint8_t index, bool value, bool update = true);
```

Sets a single output to HIGH or LOW.

**Parameters:**
- `index`: Output pin index (0 to num_bits-1)
- `value`: `true`/`HIGH` for on, `false`/`LOW` for off
- `update`: If `true`, immediately shifts out data (default). If `false`, defers update.

**Returns:** `true` if successful, `false` if index out of bounds

**Example:**
```cpp
ShiftRegister sr(5, 6, 7, 2);  // 16 outputs (2 registers)

sr.set(0, HIGH);      // Turn on output 0, update immediately
sr.set(15, HIGH);     // Turn on output 15, update immediately

// Batch update (more efficient)
sr.set(0, HIGH, false);   // Don't update yet
sr.set(1, HIGH, false);   // Don't update yet
sr.set(2, HIGH, false);   // Don't update yet
sr.push_updates();        // Update all at once
```

### get() - Read Output State

```cpp
bool get(uint8_t index) const;
```

Reads the current state of an output from internal buffer.

**Returns:** Current state (`true`/`false`), or `false` if index out of bounds

**Example:**
```cpp
if (sr.get(3)) {
    Serial.println("Output 3 is HIGH");
}
```

### setAll() - Set All Outputs

```cpp
void setAll(bool value);
```

Sets all outputs to the same state and immediately updates hardware.

**Example:**
```cpp
sr.setAll(HIGH);   // All outputs ON
delay(1000);
sr.setAll(LOW);    // All outputs OFF
```

### clear() - Clear All Outputs

```cpp
void clear();
```

Sets all outputs to LOW (equivalent to `setAll(false)`).

**Example:**
```cpp
sr.clear();  // Turn everything off
```

### push_updates() - Manual Update Control

```cpp
void push_updates(bool force = false);
```

Manually pushes queued changes to the shift registers.

**Parameters:**
- `force`: If `true`, updates hardware even if no changes pending

**Use Cases:**
- After multiple `set(index, value, false)` calls
- Periodic refresh of outputs
- Ensuring hardware synchronization

**Example:**
```cpp
// Efficient batch updates
sr.set(0, HIGH, false);
sr.set(3, HIGH, false);
sr.set(7, HIGH, false);
sr.push_updates();  // Single hardware update
```

## Query Methods

### getNumBits() / getNumRegisters()

```cpp
uint8_t getNumBits() const;
uint8_t getNumRegisters() const;
```

Returns the number of output bits or registers.

**Example:**
```cpp
Serial.print("Total outputs: ");
Serial.println(sr.getNumBits());  // e.g., 16 for 2 registers
```

### getValue()

```cpp
uint64_t getValue() const;
```

Returns the raw internal state as a 64-bit value. Useful for debugging or state saving.

### isDirty()

```cpp
bool isDirty() const;
```

Returns `true` if there are pending updates not yet sent to hardware.

## Common Usage Patterns

### Pattern 1: Simple LED Control

```cpp
#include "shift_register.h"

ShiftRegister leds(5, 6, 7, 1);  // 8 LEDs

void setup() {
    // Turn on LEDs 0, 2, 4, 6 (alternating pattern)
    for (uint8_t i = 0; i < 8; i += 2) {
        leds.set(i, HIGH);
    }
}

void loop() {
    // Blink all
    static unsigned long last = millis();
    if (millis() - last > 500) {
        last = millis();
        static bool state = false;
        state = !state;
        leds.setAll(state);
    }
}
```

### Pattern 2: Knight Rider Effect

```cpp
void loop() {
    static uint8_t pos = 0;
    static int8_t dir = 1;
    static unsigned long last = millis();
    
    if (millis() - last > 100) {
        last = millis();
        
        sr.clear();
        sr.set(pos, HIGH);
        
        pos += dir;
        if (pos == 0 || pos == 7) dir = -dir;
    }
}
```

### Pattern 3: Binary Counter Display

```cpp
void loop() {
    static uint8_t count = 0;
    static unsigned long last = millis();
    
    if (millis() - last > 1000) {
        last = millis();
        
        // Display count in binary on 8 LEDs
        for (uint8_t i = 0; i < 8; i++) {
            sr.set(i, (count >> i) & 1);
        }
        
        count++;
    }
}
```

### Pattern 4: Relay Control Bank

```cpp
ShiftRegister relays(5, 6, 7, 2);  // 16 relays

void activateZone(uint8_t zone) {
    relays.clear();                  // Turn off all relays
    if (zone < 16) {
        relays.set(zone, HIGH);      // Activate specific relay
    }
}

void multiZone(uint8_t zones[], uint8_t count) {
    relays.clear();
    for (uint8_t i = 0; i < count; i++) {
        relays.set(zones[i], HIGH, false);  // Batch mode
    }
    relays.push_updates();  // Single update
}
```

### Pattern 5: Segment Display Driver

```cpp
// 7-segment display (common cathode)
// Segments: A,B,C,D,E,F,G,DP mapped to outputs 0-7
const uint8_t DIGITS[] = {
    0b00111111,  // 0
    0b00000110,  // 1
    0b01011011,  // 2
    0b01001111,  // 3
    0b01100110,  // 4
    0b01101101,  // 5
    0b01111101,  // 6
    0b00000111,  // 7
    0b01111111,  // 8
    0b01101111   // 9
};

void displayDigit(uint8_t digit) {
    if (digit > 9) return;
    
    uint8_t pattern = DIGITS[digit];
    for (uint8_t i = 0; i < 8; i++) {
        sr.set(i, (pattern >> i) & 1, false);
    }
    sr.push_updates();
}
```

### Pattern 6: Efficient Multi-Chip Control

```cpp
ShiftRegister outputs(5, 6, 7, 4);  // 32 outputs

void setMultiple(uint8_t indices[], uint8_t count, bool state) {
    for (uint8_t i = 0; i < count; i++) {
        outputs.set(indices[i], state, false);  // Defer updates
    }
    outputs.push_updates();  // Single shift operation
}

void setup() {
    uint8_t group1[] = {0, 1, 2, 3};
    uint8_t group2[] = {8, 9, 10, 11};
    
    setMultiple(group1, 4, HIGH);
    delay(1000);
    setMultiple(group2, 4, HIGH);
}
```

## Hardware Wiring

### Single 74HC595 Shift Register

```
ESP32/Arduino          74HC595
--------------         --------
GPIO 5 (DATA)   --->   DS (14)
GPIO 6 (CLOCK)  --->   SHCP (11)
GPIO 7 (LATCH)  --->   STCP (12)
GND             --->   GND (8), OE (13)
3.3V/5V         --->   VCC (16), MR (10)

Outputs: Q0-Q7 (pins 15, 1-7)
```

**Notes:**
- OE (Output Enable) tied to GND for always-on outputs
- MR (Master Reset) tied to VCC for normal operation
- Use appropriate voltage (3.3V for ESP32, 5V for most Arduinos)

### Cascading Multiple Shift Registers

```
First 74HC595          Second 74HC595
--------------         ---------------
Q7' (9) ------------>  DS (14)
SHCP (11) <---------->  SHCP (11)  [shared clock]
STCP (12) <---------->  STCP (12)  [shared latch]
```

**Cascade Connection:**
1. Connect serial out (Q7') of first chip to DS of second chip
2. Share SHCP and STCP across all chips
3. DS of first chip connects to MCU data pin
4. Create `ShiftRegister` with `num_registers` = number of chips

**Example for 3 chips (24 outputs):**
```cpp
ShiftRegister sr(5, 6, 7, 3);  // 24 outputs total
sr.set(23, HIGH);  // Control last output of third chip
```

### Current Sourcing/Sinking

**74HC595 Limitations:**
- ~6mA per output (typ)
- ~70mA total chip current

**For Higher Current Loads:**
- Use ULN2803 Darlington array (500mA per channel)
- Use MOSFET drivers (e.g., TPIC6B595 with 150mA per channel)
- Add external transistors/MOSFETs

**Example with ULN2803:**
```
74HC595 Outputs  --->  ULN2803 Inputs  --->  High Current Loads
     Q0-Q7              IN1-IN8              (Relays, Motors, etc.)
```

## Timing Characteristics

### Update Speed

For `n` registers, each update performs:
- `n × 8` clock cycles
- 3 GPIO writes per bit (clock low, data, clock high)
- 2 latch toggles

**Approximate timing (ESP32 @ 240MHz):**
- 1 register (8 bits): ~50 µs
- 4 registers (32 bits): ~200 µs
- 8 registers (64 bits): ~400 µs

### Optimization Tips

✅ **Batch Updates:**
```cpp
// Good: Single update for multiple changes
sr.set(0, HIGH, false);
sr.set(1, HIGH, false);
sr.set(2, HIGH, false);
sr.push_updates();  // ~50 µs total

// Bad: Multiple updates
sr.set(0, HIGH);  // ~50 µs
sr.set(1, HIGH);  // ~50 µs
sr.set(2, HIGH);  // ~50 µs
// Total: ~150 µs
```

✅ **State Tracking:**
```cpp
// Use get() to avoid redundant updates
if (!sr.get(5)) {
    sr.set(5, HIGH);  // Only update if needed
}
```

## Important Notes

### ⚠️ Output Index Mapping

Outputs are indexed **LSB first** in each register:
- Single register: Index 0 = Q0, Index 7 = Q7
- Two registers: Index 0-7 = First chip Q0-Q7, Index 8-15 = Second chip Q0-Q7

### ⚠️ Power-On State

Shift register outputs are **undefined** at power-up. Always initialize:
```cpp
void setup() {
    sr.clear();  // or sr.setAll(LOW);
    // Now outputs are in known state
}
```

### ⚠️ Update Behavior

- `set()` with `update=true` (default): Immediate hardware update
- `set()` with `update=false`: Deferred update (must call `push_updates()`)
- `setAll()` and `clear()`: Always immediate update

### ⚠️ Bounds Checking

All methods perform bounds checking:
```cpp
ShiftRegister sr(5, 6, 7, 1);  // 8 outputs (0-7)
bool ok = sr.set(10, HIGH);    // Returns false, no change
bool state = sr.get(10);       // Returns false
```

### ⚠️ Maximum Registers

Limited to 8 registers (64 outputs) due to 64-bit internal storage:
```cpp
ShiftRegister sr(5, 6, 7, 100);  // Constrained to 8
Serial.println(sr.getNumRegisters());  // Prints: 8
```

For more outputs, use multiple `ShiftRegister` instances with different latch pins.

## Troubleshooting

### Outputs Not Changing

**Check:**
1. Verify power supply to shift register(s)
2. Check pin connections (data, clock, latch)
3. Ensure OE pin tied to GND (outputs enabled)
4. Verify `push_updates()` called if using deferred mode
5. Check for correct voltage levels (3.3V vs 5V)

**Debug:**
```cpp
Serial.println(sr.getValue(), BIN);  // Print internal state
Serial.println(sr.isDirty());        // Check pending updates
```

### Wrong Outputs Activating

**Check:**
1. Verify cascade wiring (Q7' to next DS)
2. Confirm `num_registers` matches hardware
3. Check for crossed data/clock lines
4. Verify all chips share clock and latch

**Test:**
```cpp
// Light one output at a time
for (uint8_t i = 0; i < sr.getNumBits(); i++) {
    sr.clear();
    sr.set(i, HIGH);
    delay(500);
}
```

### Flickering or Noise

**Check:**
1. Add decoupling capacitors (0.1µF near each chip)
2. Use shorter wires (especially for clock/data)
3. Reduce clock speed if needed (add delays)
4. Ensure common ground between MCU and shift registers

### Inconsistent Behavior

**Check:**
1. Verify power supply can handle load
2. Check for loose connections
3. Ensure MR (reset) pin tied HIGH
4. Add pull-up resistor on OE if floating

## Performance Characteristics

- **Memory per instance:** ~24 bytes
- **Update time:** ~50 µs per register (8 bits)
- **CPU overhead:** Minimal (blocking during update only)
- **Maximum update rate:** ~20 kHz (single register)

## Best Practices

✅ **DO:**
- Use batch updates for multiple changes (`update=false`, then `push_updates()`)
- Initialize to known state in `setup()` with `clear()` or `setAll()`
- Use `get()` to track state and avoid redundant updates
- Add decoupling capacitors (0.1µF) near each shift register
- Tie unused outputs to GND through resistors if driving LEDs

❌ **DON'T:**
- Exceed current limits (add drivers for high-current loads)
- Forget to call `push_updates()` when using deferred mode
- Rely on power-on state (always initialize)
- Use excessively long wires (causes noise and timing issues)
- Access out-of-bounds indices (check return values)

## Example: Complete LED Matrix Controller

```cpp
#include "shift_register.h"

// 8x8 LED matrix using 2 shift registers
// One for rows (cathodes), one for columns (anodes)
ShiftRegister rows(5, 6, 7, 1);
ShiftRegister cols(8, 9, 10, 1);

uint8_t display[8] = {
    0b00111100,  // Smiley face
    0b01000010,
    0b10100101,
    0b10000001,
    0b10100101,
    0b10011001,
    0b01000010,
    0b00111100
};

void setup() {
    rows.clear();
    cols.clear();
}

void loop() {
    // Scan rows at high speed (persistence of vision)
    for (uint8_t row = 0; row < 8; row++) {
        rows.clear();
        
        // Set column data
        uint8_t pattern = display[row];
        for (uint8_t col = 0; col < 8; col++) {
            cols.set(col, (pattern >> col) & 1, false);
        }
        cols.push_updates();
        
        // Activate row
        rows.set(row, HIGH);
        
        delayMicroseconds(500);  // ~2ms total refresh rate
    }
}
```

## Advanced: State Persistence

```cpp
#include <Preferences.h>

Preferences prefs;
ShiftRegister sr(5, 6, 7, 2);

void saveState() {
    prefs.begin("shift-reg", false);
    prefs.putULong64("state", sr.getValue());
    prefs.end();
}

void loadState() {
    prefs.begin("shift-reg", true);
    uint64_t state = prefs.getULong64("state", 0);
    prefs.end();
    
    // Restore each bit
    for (uint8_t i = 0; i < sr.getNumBits(); i++) {
        sr.set(i, (state >> i) & 1, false);
    }
    sr.push_updates();
}
```

## License

This component is subject to the main project license (see `LICENSE`).

---

**Need help?** Check the examples above or review the test suite for detailed usage scenarios.
