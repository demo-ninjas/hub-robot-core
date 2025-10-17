#ifndef HUB_SHIFT_REGISTER_H
#define HUB_SHIFT_REGISTER_H

#include <Arduino.h>

class ShiftRegister {
    private: 
        uint8_t data_pin;
        uint8_t clock_pin;
        uint8_t latch_pin;
        uint8_t num_bits;
        uint64_t val;  // Changed from unsigned long to support up to 8 registers (64 bits)
        bool dirty;
        void update();
    public:
        ShiftRegister(uint8_t data_pin, uint8_t clock_pin, uint8_t latch_pin, uint8_t num_registers = 1);
        bool set(uint8_t index, bool value, bool update = true);
        void setAll(bool value);
        void clear();
        void push_updates(bool force = false);
        
        // Query methods for testing and debugging
        bool get(uint8_t index) const;
        uint8_t getNumBits() const { return num_bits; }
        uint8_t getNumRegisters() const { return num_bits / 8; }
        uint64_t getValue() const { return val; }
        bool isDirty() const { return dirty; }
};

#endif  // HUB_SHIFT_REGISTER_H