#ifndef HUB_SHIFT_REGISTER_H
#define HUB_SHIFT_REGISTER_H

#include <Arduino.h>

class ShiftRegister {
    private: 
        uint8_t data_pin;
        uint8_t clock_pin;
        uint8_t latch_pin;
        uint8_t num_bits;
        unsigned long val;
        bool dirty;
        void update();
    public:
        ShiftRegister(uint8_t data_pin, uint8_t clock_pin, uint8_t latch_pin, uint8_t num_registers = 1);
        void set(uint8_t index, bool value, bool update = true);
        void setAll(bool value);
        void clear();
        void push_updates(bool force = false);
};

#endif  // HUB_SHIFT_REGISTER_H