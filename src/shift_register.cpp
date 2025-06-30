
#include "shift_register.h"

ShiftRegister::ShiftRegister(uint8_t data_pin, uint8_t clock_pin, uint8_t latch_pin, uint8_t num_registers) {
    num_registers = constrain(num_registers, 1, 8);
    this->data_pin = data_pin;
    this->clock_pin = clock_pin;
    this->latch_pin = latch_pin;
    this->num_bits = num_registers * 8;
    this->val = 0;
    this->dirty = false;
    
    pinMode(data_pin, OUTPUT);
    pinMode(clock_pin, OUTPUT);
    pinMode(latch_pin, OUTPUT);
}

void ShiftRegister::set(uint8_t index, bool value, bool update) {
    if (index >= this->num_bits) {
        return;
    }

    if (value) {
        this->val |= (1 << index);
    } else {
        this->val &= ~(1 << index);
    }

    this->dirty = true;

    if (update) {
        this->update();
    }
}

void ShiftRegister::setAll(bool value) {
    if (value) {
        this->val = (1 << this->num_bits) - 1; // Set all bits to 1
    } else {
        this->val = 0; // Set all bits to 0
    }
    this->dirty = true;
    this->update();
}

void ShiftRegister::clear() {
    this->setAll(0);
}

void ShiftRegister::push_updates(bool force) {
    if (!this->dirty && !force) {
        return;
    }
    this->update();
}

void ShiftRegister::update() {
    digitalWrite(this->latch_pin, LOW);
    digitalWrite(this->data_pin, LOW);
    // Assume MSB first...
    for (int i = 0; i < this->num_bits; i++) {
        digitalWrite(this->clock_pin, LOW);
        if (this->val & (1 << (this->num_bits - 1 - i))) {
            digitalWrite(this->data_pin, HIGH);
        } else {
            digitalWrite(this->data_pin, LOW);
        }
        digitalWrite(this->clock_pin, HIGH);
        digitalWrite(this->data_pin, LOW);  // Feels safer to enter the next clock cycle with data low, but probably not necessary ;p
    }
    digitalWrite(this->latch_pin, HIGH);
}
