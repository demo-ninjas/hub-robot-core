/**
 * @file dc_motor.h
 * @brief Simple DC motor driver abstraction for an H-bridge (e.g. L298, TB6612)
 *
 * Features / design notes:
 * - Signed speed value in range [-255, 255]; sign encodes direction.
 * - Internally skips redundant writes when setSpeed is called with unchanged value
 *   to reduce ISR latency / CPU usage on tight loops.
 * - Provides helper accessors for direction (+1,0,-1) and magnitude (0..255).
 * - stop() performs a “coast” (both IN pins LOW). brake() performs an “active brake”
 *   (both IN pins HIGH) when supported by the H-bridge.
 * - All pin writes are performed only if necessary to avoid superfluous I/O.
 *
 * This class assumes an enable PWM pin plus two direction pins. The PWM write uses
 * analogWrite; on ESP32 the default Arduino core maps this to LEDC automatically.
 */

#ifndef HUB_DC_MOTOR_H
#define HUB_DC_MOTOR_H

#include <Arduino.h>

class DCMotor {
    public:
        static constexpr int16_t kMaxSpeed = 255; // absolute maximum speed value

        /**
         * @brief Construct a motor driver given enable + direction pins.
         * @param en_pin PWM capable enable pin.
         * @param in1_pin Direction pin 1.
         * @param in2_pin Direction pin 2.
         */
        DCMotor(uint8_t en_pin, uint8_t in1_pin, uint8_t in2_pin);

        /**
         * @brief Set signed speed. Negative => reverse, positive => forward, 0 => stop (coast).
         * @param speed Desired speed in range [-255, 255]; values are constrained.
         */
        void setSpeed(int speed);

        /**
         * @brief Immediately stop (coast). Equivalent to setSpeed(0) but forces pin state update.
         */
        void stop();

        /**
         * @brief Active brake (both direction pins HIGH, PWM = 0). Some H-bridges short the motor terminals.
         */
        void brake();

        /**
         * @brief Get last commanded signed speed (range [-255,255]).
         */
        int16_t getSpeed() const { return speed_; }

        /**
         * @brief Get absolute speed magnitude (0..255).
         */
        uint8_t getMagnitude() const { return static_cast<uint8_t>(speed_ < 0 ? -speed_ : speed_); }

        /**
         * @brief Direction helper: -1 reverse, 0 stopped, +1 forward.
         */
        int8_t getDirection() const {
            if (speed_ > 0) return 1;
            if (speed_ < 0) return -1;
            return 0;
        }

    private:
        uint8_t en_pin_;
        uint8_t in1_pin_;
        uint8_t in2_pin_;
        int16_t speed_;       // signed speed [-255,255]
        uint8_t last_pwm_;    // last PWM magnitude applied
        int8_t last_dir_;     // cached direction (-1,0,1)

        void applyPinsForSpeed(int16_t new_speed, bool forcePins);
};

#endif  // HUB_DC_MOTOR_H