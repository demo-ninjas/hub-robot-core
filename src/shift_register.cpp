
#include "shift_register.h"

ShiftRegister::ShiftRegister(uint8_t data_pin, uint8_t clock_pin, uint8_t latch_pin, uint8_t num_registers) {
    // Constrain to valid range: 1-8 registers (8-64 bits)
    num_registers = constrain(num_registers, 1, 8);
    
    this->data_pin = data_pin;
    this->clock_pin = clock_pin;
    this->latch_pin = latch_pin;
    this->num_bits = num_registers * 8;
    this->val = 0;
    this->dirty = false;
    
    // Initialize pins
    pinMode(data_pin, OUTPUT);
    pinMode(clock_pin, OUTPUT);
    pinMode(latch_pin, OUTPUT);
    
    // Ensure outputs start in known state (all LOW)
    digitalWrite(data_pin, LOW);
    digitalWrite(clock_pin, LOW);
    digitalWrite(latch_pin, LOW);
}

bool ShiftRegister::set(uint8_t index, bool value, bool update) {
    // Bounds check - return false if out of range
    if (index >= this->num_bits) {
        return false;
    }

    // Use 64-bit literal to avoid undefined behavior
    uint64_t mask = 1ULL << index;
    
    if (value) {
        this->val |= mask;
    } else {
        this->val &= ~mask;
    }

    this->dirty = true;

    if (update) {
        this->update();
    }
    
    return true;
}

void ShiftRegister::setAll(bool value) {
    if (value) {
        // Safe way to set all bits without undefined behavior
        if (this->num_bits == 64) {
            this->val = 0xFFFFFFFFFFFFFFFFULL;
        } else {
            this->val = (1ULL << this->num_bits) - 1;
        }
    } else {
        this->val = 0;
    }
    this->dirty = true;
    this->update();
}

void ShiftRegister::clear() {
    this->setAll(false);
}

void ShiftRegister::push_updates(bool force) {
    if (!this->dirty && !force) {
        return;
    }
    this->update();
}

bool ShiftRegister::get(uint8_t index) const {
    if (index >= this->num_bits) {
        return false;
    }
    return (this->val & (1ULL << index)) != 0;
}

void ShiftRegister::update() {
    // Begin latch sequence
    digitalWrite(this->latch_pin, LOW);
    
    // Shift out data MSB first
    for (int i = 0; i < this->num_bits; i++) {
        digitalWrite(this->clock_pin, LOW);
        
        // Check bit at position (num_bits - 1 - i) for MSB first
        if (this->val & (1ULL << (this->num_bits - 1 - i))) {
            digitalWrite(this->data_pin, HIGH);
        } else {
            digitalWrite(this->data_pin, LOW);
        }
        
        // Clock pulse to shift in the bit
        digitalWrite(this->clock_pin, HIGH);
    }
    
    // Final state: leave clock high, data low for consistency
    digitalWrite(this->data_pin, LOW);
    
    // Complete latch sequence - transfer data to outputs
    digitalWrite(this->latch_pin, HIGH);
    
    // Clear dirty flag after successful update
    this->dirty = false;
}
