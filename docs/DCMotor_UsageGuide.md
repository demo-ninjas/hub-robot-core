# DCMotor Class - Usage Guide

## Quick Start

```cpp
#include "dc_motor.h"

// Enable pin must support PWM (ESP32: any LEDC capable GPIO). Direction pins are standard GPIO.
DCMotor motor(/*EN=*/5, /*IN1=*/6, /*IN2=*/7);

void setup() {
    Serial.begin(115200);
    // Spin forward at ~50% duty
    motor.setSpeed(128); // Range: -255 .. 255
}

void loop() {
    // Change direction every 2 seconds (example pattern)
    static unsigned long last = millis();
    if (millis() - last > 2000) {
        last = millis();
        motor.setSpeed(-motor.getSpeed()); // Flip sign toggles direction
    }
}
```

## Overview
The `DCMotor` class abstracts a simple DC motor driven by a typical dual H-bridge or half-bridge driver (e.g. L298N, TB6612FNG, DRV8833) using three pins:
- Enable (PWM) pin: controls speed via duty cycle.
- IN1 and IN2 direction pins: control forward, reverse, coast, or brake.

Speed is a signed integer in the range `[-255, 255]`:
- Positive: forward
- Negative: reverse
- Zero: coast (free-wheel)

Active braking (electrical short of motor leads through the bridge) is supported via `brake()`.

## Constructor
```cpp
DCMotor(uint8_t en_pin, uint8_t in1_pin, uint8_t in2_pin);
```
### Parameters
- `en_pin`: PWM capable pin for speed (ESP32 Arduino core automatically maps `analogWrite` to LEDC).
- `in1_pin`: First direction pin.
- `in2_pin`: Second direction pin.

Pins are set to OUTPUT and initial state is coast (both low, PWM = 0).

## Core API
```cpp
void setSpeed(int speed);    // Signed speed -255..255 (clamped)
void stop();                  // Coast (equivalent to setSpeed(0) but forces pins)
void brake();                 // Active brake (both IN pins HIGH, PWM=0)
int16_t getSpeed() const;     // Last commanded signed speed
uint8_t getMagnitude() const; // Absolute speed (0..255)
int8_t getDirection() const;  // -1 reverse, 0 stopped, +1 forward
```

### setSpeed(int speed)
- Clamps input to `[-255, 255]`.
- Skips redundant pin/PWM writes if the value is unchanged (reduces I/O overhead in tight loops).
- Direction changes cause only the direction pins to update; PWM is reused if magnitude unchanged.

### stop()
- Forces coast state (both direction pins LOW, PWM=0) even if already stopped to guarantee pin outputs.

### brake()
- Sets both direction pins HIGH and PWM=0. On most H-bridges this actively brakes the motor by shorting the terminals (rapid deceleration). Not suitable for delicate mechanisms without testing.

### Direction vs Magnitude
Use `getDirection()` to avoid manually interpreting sign; use `getMagnitude()` for speed scaling logic.

## Behavior States
| State       | IN1 | IN2 | PWM        | Description              |
|-------------|-----|-----|------------|--------------------------|
| Forward     | HIGH| LOW | >0 (duty)  | Motor spins forward      |
| Reverse     | LOW | HIGH| >0 (duty)  | Motor spins reverse      |
| Coast (0)   | LOW | LOW | 0          | Free-run (minimal drag)  |
| Brake       | HIGH| HIGH| 0          | Active electrical brake  |

## Example Patterns
### Soft Start / Ramp
```cpp
void rampTo(DCMotor &m, int target, int step = 5, unsigned delay_ms = 15) {
    target = constrain(target, -255, 255);
    int current = m.getSpeed();
    while (current != target) {
        if (current < target) current = min(current + step, target);
        else current = max(current - step, target);
        m.setSpeed(current);
        delay(delay_ms); // Simple blocking ramp; consider millis()-based non-blocking design
    }
}
```

### Toggle Direction with Button
```cpp
#include "button.h"
Button dirButton(12);
DCMotor motor(5,6,7);

void setup() {
    dirButton.onPressed([](long d){
        int sp = motor.getSpeed();
        if (sp == 0) motor.setSpeed(160); // start forward
        else motor.setSpeed(-sp);         // flip direction
    });
}

void loop() {
    dirButton.tick();
}
```

### Emergency Brake
```cpp
void emergencyStop(DCMotor &m) {
    m.brake(); // Fast halt
    // Optionally follow with coast after a short delay to relieve stress
    delay(100);
    m.stop();
}
```

### Periodic Load Sampling (Pseudo)
```cpp
void loop() {
    // ... other logic
    static unsigned long lastSample = 0;
    if (millis() - lastSample > 500) {
        lastSample = millis();
        uint8_t mag = motor.getMagnitude();
        int8_t dir = motor.getDirection();
        Serial.printf("Motor: dir=%d mag=%u\n", dir, mag);
    }
}
```

## Wiring Reference
Typical H-bridge (e.g., TB6612FNG):
```
 MCU Pin (EN) ---- PWM Enable ---- AIN1 IN1 -> motor terminal A
 MCU Pin (IN1) --- Direction ------ AIN2 IN2 -> motor terminal B
 MCU Pin (IN2) --- Direction ------
 Motor supply (VM) and GND per driver specs
 Ensure common ground between MCU and driver
```
Notes:
- Some boards label IN pins differently; ensure mapping matches forward/reverse expectations.
- EN pin may need to be tied HIGH if not using PWM (in which case pass a dummy PWM pin or modify class for always-on). Current version assumes PWM usage.

## Best Practices
✅ Use ramping for large inertial loads to avoid high stall current.
✅ Periodically verify temperature of the driver under sustained high duty cycles.
✅ Keep supply wiring short and adequately rated; add a bulk capacitor near the driver (e.g., 470µF).
✅ Use `brake()` sparingly; repeated rapid braking can cause voltage spikes.
✅ Debounce control inputs (use existing `Button` class) when changing speed direction frequently.

❌ Avoid instantly flipping from +255 to -255; ramp instead.
❌ Don’t rely solely on software for overcurrent protection; use driver and fuse hardware.
❌ Avoid long blocking `delay()` inside control loops; prefer non-blocking scheduling.

## Edge Cases & Misuse Handling
| Scenario | Outcome |
|----------|---------|
| `setSpeed(500)` | Clamped to 255 |
| `setSpeed(-999)`| Clamped to -255 |
| Repeated `setSpeed(100)` | No redundant pin/PWM updates |
| `brake()` then `setSpeed(140)` | Direction + PWM reapplied correctly |
| `stop()` after `brake()` | Restores coast (LOW/LOW, PWM=0) |
| Calling `brake()` repeatedly | Idempotent (pin state unchanged) |

## Performance Notes
- Operation is O(1) per call.
- Redundant `setSpeed()` does zero I/O (fast path). Useful in loops recalculating speed but unchanged.
- On ESP32, `analogWrite` uses LEDC; consider customizing channel/frequency if needed (extension idea).

## Unit Tests
Reference tests in `test/test_dc_motor.cpp`:
- Validate direction logic, clamping, redundancy optimization, braking, transitions.
- Use mocked pin and PWM arrays (no hardware reliance).

To execute (after installing PlatformIO):
```bash
pio test --without-uploading
```

## Extending the Class
Potential additions (not yet implemented):
- Configurable PWM frequency / LEDC channel selection (ESP32-specific).
- Optional acceleration limiting (slew rate).
- Current sensing integration (using ADC for closed-loop control).
- Fault input monitoring (ENA/B flags from smart drivers).

## Troubleshooting
| Issue | Check |
|-------|-------|
| Motor does not spin | Verify supply voltage; confirm EN PWM > 0; direction pins not both HIGH unless braking |
| Only one direction works | Swap motor leads or verify IN1/IN2 wiring |
| Excessive heat | Lower duty cycle; add cooling; confirm motor rated voltage |
| Noise / resets | Add decoupling capacitor; separate motor supply from MCU 5V rail |
| Sudden stops unexpected | Ensure `brake()` not called unintentionally; search codebase for `brake()` invocations |

## Complete Example: Ramped Speed Controller
```cpp
#include "dc_motor.h"

DCMotor motor(5,6,7);
int targetSpeed = 0; // -255..255

void setup() { Serial.begin(115200); }

void loop() {
    // Example: sweep from -255 to 255
    static int step = 5;
    static unsigned long last = millis();
    if (millis() - last > 40) { // every 40ms
        last = millis();
        targetSpeed += step;
        if (targetSpeed > 255) { targetSpeed = 255; step = -5; }
        if (targetSpeed < -255) { targetSpeed = -255; step = 5; }
        motor.setSpeed(targetSpeed); // redundancy optimization if same value
    }

    // Emergency condition example
    if (false /* replace with sensor check */) {
        motor.brake();
    }
}
```

## License
This component is subject to the main project license (see `LICENSE`).

---
Feel free to adapt and extend; contributions welcome!
