
#include "dc_motor.h"

DCMotor::DCMotor(uint8_t en_pin, uint8_t in1_pin, uint8_t in2_pin) {
    this->en_pin = en_pin;
    this->in1_pin = in1_pin;
    this->in2_pin = in2_pin;
    this->speed = 0;

    pinMode(en_pin, OUTPUT);
    pinMode(in1_pin, OUTPUT);
    pinMode(in2_pin, OUTPUT);
}

void DCMotor::setSpeed(int speed) {
    speed = constrain(speed, -255, 255);
    if (speed > 0) {
        digitalWrite(this->in1_pin, HIGH);
        digitalWrite(this->in2_pin, LOW);
    } else if (speed < 0) {
        digitalWrite(this->in1_pin, LOW);
        digitalWrite(this->in2_pin, HIGH);
    } else {
        digitalWrite(this->in1_pin, LOW);
        digitalWrite(this->in2_pin, LOW);
    }
    analogWrite(this->en_pin, abs(speed));
    this->speed = speed;
}

void DCMotor::stop() {
    digitalWrite(this->in1_pin, LOW);
    digitalWrite(this->in2_pin, LOW);
    analogWrite(this->en_pin, 0);
    this->speed = 0;
}

uint8_t DCMotor::getSpeed() {
    return this->speed;
}