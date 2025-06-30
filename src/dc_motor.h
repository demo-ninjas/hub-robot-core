#ifndef HUB_DC_MOTOR_H
#define HUB_DC_MOTOR_H

#include <Arduino.h>

class DCMotor {
    private:
        uint8_t en_pin;
        uint8_t in1_pin;
        uint8_t in2_pin;
        uint8_t speed;
        
    public:
        DCMotor(uint8_t en_pin, uint8_t in1_pin, uint8_t in2_pin);
        void setSpeed(int speed);
        void stop();
        uint8_t getSpeed();
};

#endif  // HUB_DC_MOTOR_H