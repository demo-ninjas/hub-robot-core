
#include "dc_motor.h"

DCMotor::DCMotor(uint8_t en_pin, uint8_t in1_pin, uint8_t in2_pin)
        : en_pin_(en_pin), in1_pin_(in1_pin), in2_pin_(in2_pin), speed_(0), last_pwm_(0), last_dir_(0) {
    pinMode(en_pin_, OUTPUT);
    pinMode(in1_pin_, OUTPUT);
    pinMode(in2_pin_, OUTPUT);
    // Initialize to coast state
    digitalWrite(in1_pin_, LOW);
    digitalWrite(in2_pin_, LOW);
    analogWrite(en_pin_, 0);
}

void DCMotor::applyPinsForSpeed(int16_t new_speed, bool forcePins) {
    int8_t dir = 0;
    if (new_speed > 0) dir = 1; else if (new_speed < 0) dir = -1;

    // Only update direction pins if direction changed or forced.
    if (forcePins || dir != last_dir_) {
        if (dir > 0) {
            digitalWrite(in1_pin_, HIGH);
            digitalWrite(in2_pin_, LOW);
        } else if (dir < 0) {
            digitalWrite(in1_pin_, LOW);
            digitalWrite(in2_pin_, HIGH);
        } else { // coast
            digitalWrite(in1_pin_, LOW);
            digitalWrite(in2_pin_, LOW);
        }
        last_dir_ = dir;
    }

    uint8_t pwm = static_cast<uint8_t>(new_speed < 0 ? -new_speed : new_speed);
    if (pwm != last_pwm_) {
        analogWrite(en_pin_, pwm);
        last_pwm_ = pwm;
    }
    speed_ = new_speed;
}

void DCMotor::setSpeed(int speed) {
    int16_t constrained = static_cast<int16_t>(constrain(speed, -kMaxSpeed, kMaxSpeed));
    // Fast path: if identical, skip writes.
    if (constrained == speed_)
        return;
    applyPinsForSpeed(constrained, false);
}

void DCMotor::stop() {
    // Force coast even if already stopped to guarantee pin state.
    applyPinsForSpeed(0, true);
}

void DCMotor::brake() {
    // Active brake: both direction pins HIGH (depending on driver this may short motor terminals).
    digitalWrite(in1_pin_, HIGH);
    digitalWrite(in2_pin_, HIGH);
    analogWrite(en_pin_, 0);
    speed_ = 0;
    last_pwm_ = 0;
    last_dir_ = 0; // treat as stopped direction-wise
}