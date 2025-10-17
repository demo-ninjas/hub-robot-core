# Button Class - Usage Guide

## Quick Start

```cpp
#include "button.h"

// Create a button on pin 5 with default settings
Button myButton(5);

void setup() {
    // Set up callback for single press
    myButton.onPressed([](long duration) {
        Serial.println("Button pressed!");
    });
}

void loop() {
    // Must call tick() regularly (at least every 10-20ms)
    myButton.tick();
}
```

## Constructor Parameters

```cpp
Button(int pin, 
       unsigned long debounce_delay_ms = 25,
       bool use_interrupts = true,
       bool btn_pulls_to_ground = true,
       unsigned long long_press_time_ms = 800,
       unsigned long double_press_time_ms = 300);
```

### Parameters Explained

- **pin**: GPIO pin number where button is connected
- **debounce_delay_ms**: Time (ms) the button state must be stable before registering a change
  - Default: 25ms (good for most switches)
  - Increase for noisy switches (50-100ms)
  - Decrease for very responsive feel (10-15ms)
  
- **use_interrupts**: Whether to use hardware interrupts
  - `true`: More efficient, pin must support interrupts (default)
  - `false`: Use polling mode, works on any pin
  
- **btn_pulls_to_ground**: Button wiring configuration
  - `true`: Button connects pin to GND when pressed (with INPUT_PULLUP) - most common
  - `false`: Button connects pin to VCC when pressed (with INPUT_PULLDOWN)
  
- **long_press_time_ms**: Duration (ms) to hold button for long press
  - Default: 800ms
  - Common range: 500-2000ms
  
- **double_press_time_ms**: Max time (ms) between presses for double-press
  - Default: 300ms
  - Common range: 200-500ms

## Callback Functions

### onPressed - Single Press
Triggered when button is pressed and released (not long press or double press).

```cpp
myButton.onPressed([](long duration) {
    Serial.print("Pressed for ");
    Serial.print(duration);
    Serial.println(" ms");
});
```

**When it fires:**
- After button is released
- Press duration < long_press_time_ms
- No second press within double_press_time_ms

### onDoublePressed - Double Press
Triggered when button is pressed twice quickly.

```cpp
myButton.onDoublePressed([](long interval) {
    Serial.print("Double press! Interval: ");
    Serial.print(interval);
    Serial.println(" ms");
});
```

**When it fires:**
- After second button release
- Second press started within double_press_time_ms of first release
- Takes precedence over single press

### onLongPressed - Long Press
Triggered when button is held down for extended time.

```cpp
myButton.onLongPressed([](long duration) {
    Serial.print("Long press! Duration: ");
    Serial.print(duration);
    Serial.println(" ms");
});
```

**When it fires:**
- After button is released
- Press duration >= long_press_time_ms
- Takes precedence over single and double press

### onDown - Button Pressed Down
Triggered immediately when button goes down (after debounce).

```cpp
myButton.onDown([]() {
    Serial.println("Button DOWN");
});
```

**When it fires:**
- Immediately when button is pressed (after debounce)
- Every time button goes from UP to DOWN state

### onUp - Button Released
Triggered immediately when button is released (after debounce).

```cpp
myButton.onUp([]() {
    Serial.println("Button UP");
});
```

**When it fires:**
- Immediately when button is released (after debounce)
- Every time button goes from DOWN to UP state

## State Query Methods

### isDown() / isUp()
Check current button state.

```cpp
if (myButton.isDown()) {
    Serial.println("Button is currently pressed");
}

if (myButton.isUp()) {
    Serial.println("Button is currently released");
}
```

### getTimeInCurrentState()
Get how long button has been in current state.

```cpp
if (myButton.isDown()) {
    unsigned long hold_time = myButton.getTimeInCurrentState();
    Serial.print("Button held for ");
    Serial.print(hold_time);
    Serial.println(" ms");
}
```

### getTimeInLastState()
Get how long button was in previous state.

```cpp
myButton.onUp([]() {
    unsigned long press_duration = myButton.getTimeInLastState();
    Serial.print("Was pressed for ");
    Serial.print(press_duration);
    Serial.println(" ms");
});
```

## Common Usage Patterns

### Pattern 1: Simple Toggle
```cpp
bool led_state = false;

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    
    myButton.onPressed([](long duration) {
        led_state = !led_state;
        digitalWrite(LED_BUILTIN, led_state);
    });
}
```

### Pattern 2: Menu Navigation
```cpp
int menu_item = 0;
const int MAX_ITEMS = 5;

void setup() {
    // Single press: Next item
    myButton.onPressed([](long duration) {
        menu_item = (menu_item + 1) % MAX_ITEMS;
        displayMenu(menu_item);
    });
    
    // Double press: Select item
    myButton.onDoublePressed([](long interval) {
        selectMenuItem(menu_item);
    });
    
    // Long press: Exit menu
    myButton.onLongPressed([](long duration) {
        exitMenu();
    });
}
```

### Pattern 3: Multiple Buttons
```cpp
Button button1(5);
Button button2(6);
Button button3(7);

void setup() {
    button1.onPressed([](long d) { action1(); });
    button2.onPressed([](long d) { action2(); });
    button3.onPressed([](long d) { action3(); });
}

void loop() {
    button1.tick();
    button2.tick();
    button3.tick();
}
```

### Pattern 4: Combination Press
```cpp
Button btnA(5);
Button btnB(6);

void setup() {
    btnA.onDown([]() {
        if (btnB.isDown()) {
            // Both buttons pressed!
            specialAction();
        }
    });
}
```

### Pattern 5: Hold-to-Repeat
```cpp
void loop() {
    myButton.tick();
    
    if (myButton.isDown()) {
        unsigned long hold_time = myButton.getTimeInCurrentState();
        
        // Start repeating after 1 second
        if (hold_time > 1000) {
            // Repeat every 100ms
            if ((hold_time - 1000) % 100 == 0) {
                incrementValue();
            }
        }
    }
}
```

### Pattern 6: Power Button Behavior
```cpp
enum PowerState { ON, SLEEP, OFF };
PowerState state = ON;

void setup() {
    Button powerBtn(5, 25, true, true, 3000, 300); // 3s long press
    
    // Short press: Toggle sleep
    powerBtn.onPressed([](long duration) {
        state = (state == ON) ? SLEEP : ON;
    });
    
    // Long press: Power off
    powerBtn.onLongPressed([](long duration) {
        state = OFF;
        shutdown();
    });
}
```

## Important Notes

### ⚠️ Must Call tick() Regularly

The button **requires** regular calls to `tick()` in your main loop:

```cpp
void loop() {
    myButton.tick();  // Call this every loop iteration
    
    // Rest of your code...
}
```

**Interrupt mode:** Can call `tick()` less frequently (10-100 Hz)
**Polling mode:** Must call `tick()` at least 2× faster than debounce time (e.g., every 10ms for 25ms debounce)

### ⚠️ Callback Timing

- `onDown()` and `onUp()` fire immediately (after debounce)
- `onPressed()`, `onDoublePressed()`, and `onLongPressed()` fire after button is released
- Single press has additional delay (double_press_time_ms) to distinguish from double press

### ⚠️ Interrupt Pin Requirements

When `use_interrupts = true`, the pin must support hardware interrupts:
- **ESP32**: Most GPIO pins support interrupts
- **ESP8266**: Most GPIO pins support interrupts (avoid GPIO16)
- **Arduino Uno**: Pins 2 and 3 only
- **Arduino Mega**: Pins 2, 3, 18, 19, 20, 21

If unsure, use `use_interrupts = false` for polling mode.

### ⚠️ Button Wiring

**Pull-to-Ground (most common):**
```
Button Pin ---[Switch]--- GND
                |
            [Pull-up resistor to VCC]
```
Use: `Button btn(pin, 25, true, true);`

**Pull-to-High:**
```
Button Pin ---[Switch]--- VCC
                |
            [Pull-down resistor to GND]
```
Use: `Button btn(pin, 25, true, false);`

## Troubleshooting

### Button not responding
1. Check pin number is correct
2. Verify `tick()` is called regularly
3. Try increasing debounce time: `Button btn(pin, 50);`
4. Check button wiring matches `btn_pulls_to_ground` setting

### Multiple triggers / bouncing
1. Increase debounce time: `Button btn(pin, 50);`
2. Check for electrical noise / poor connections
3. Add hardware debounce capacitor (0.1µF across button)

### Long press fires too soon/late
Adjust `long_press_time_ms`:
```cpp
Button btn(pin, 25, true, true, 1500); // 1.5 second long press
```

### Double press not detecting
1. Press faster (within 300ms by default)
2. Increase timeout: `Button btn(pin, 25, true, true, 800, 500);`
3. Verify both presses are longer than debounce time

### Interrupts not working
1. Verify pin supports interrupts
2. Try polling mode: `Button btn(pin, 25, false);`
3. Check for interrupt conflicts with other libraries

## Best Practices

✅ **DO:**
- Call `tick()` in every loop iteration
- Use interrupts when possible for efficiency
- Set appropriate debounce time for your switch (test and adjust)
- Use `const` when passing Button to functions that don't modify it

❌ **DON'T:**
- Do heavy processing inside callbacks (keep them fast)
- Forget to call `tick()` regularly
- Use delays in callbacks (blocks tick processing)
- Create Button objects dynamically without proper cleanup

## Performance Characteristics

- **Memory:** ~72 bytes per Button instance
- **CPU (interrupt mode):** Nearly zero between state changes
- **CPU (polling mode):** ~10 µs per tick() call (negligible)
- **Response time:** debounce_delay_ms + tick() interval

## Example: Complete Application

```cpp
#include "button.h"

// Hardware
const int BUTTON_PIN = 5;
const int LED_PIN = LED_BUILTIN;

// State
bool led_on = false;
int brightness = 128;

// Button instance
Button myButton(BUTTON_PIN);

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    
    // Single press: Toggle LED
    myButton.onPressed([](long duration) {
        led_on = !led_on;
        analogWrite(LED_PIN, led_on ? brightness : 0);
        Serial.println(led_on ? "LED ON" : "LED OFF");
    });
    
    // Double press: Increase brightness
    myButton.onDoublePressed([](long interval) {
        brightness = min(255, brightness + 32);
        if (led_on) analogWrite(LED_PIN, brightness);
        Serial.print("Brightness: ");
        Serial.println(brightness);
    });
    
    // Long press: Reset brightness
    myButton.onLongPressed([](long duration) {
        brightness = 128;
        if (led_on) analogWrite(LED_PIN, brightness);
        Serial.println("Brightness reset");
    });
}

void loop() {
    myButton.tick();  // Essential!
    
    // Other application code...
}
```
