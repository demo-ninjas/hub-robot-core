
#include "button.h"

Button::Button(int pin, unsigned long debounce_delay_ms, bool use_interrupts, bool btn_pulls_to_ground, unsigned long long_press_time_ms, unsigned long double_press_time_ms) {
    this->pin = pin;
    this->state = BUTTON_UP;
    this->time_entered_state = millis();
    this->debounce_delay_ms = debounce_delay_ms;
    this->state_from_interrupt = BUTTON_UP;
    this->last_debounce_time = 0;
    this->duration_in_previous_state = 0;
    this->time_of_last_up = 0;
    this->use_interrupts = use_interrupts;
    this->btn_pulls_to_ground = btn_pulls_to_ground;
    this->long_press_time_ms = long_press_time_ms;
    this->double_press_time_ms = double_press_time_ms;
    this->state_handled = true;
    this->last_press_time = 0;
    this->debounce_state = BUTTON_UP;


    if (btn_pulls_to_ground) {
        pinMode(pin, INPUT_PULLUP);
    } else {
        pinMode(pin, INPUT_PULLDOWN);
    }
    
    if (use_interrupts) {
        attachInterruptArg(digitalPinToInterrupt(pin), [](void* arg) {
            static_cast<Button*>(arg)->handleOnDownInterrupt();
        }, this, btn_pulls_to_ground ? FALLING : RISING);
        attachInterruptArg(digitalPinToInterrupt(pin), [](void* arg) {
            static_cast<Button*>(arg)->handleOnUpInterrupt();
        }, this, btn_pulls_to_ground ? RISING : FALLING);
    }
}

Button::~Button() {
    if (this->use_interrupts) {
        detachInterrupt(digitalPinToInterrupt(this->pin));
    }
}

void Button::handleOnDownInterrupt() {
    if (this->state_from_interrupt == BUTTON_DOWN) {
        return; // Already down, no need to handle again
    }

    this->state_from_interrupt = BUTTON_DOWN;
}

void Button::handleOnUpInterrupt() {
    if (this->state_from_interrupt == BUTTON_UP) {
        return; // Already up, no need to handle again
    }

    this->state_from_interrupt = BUTTON_UP;
}

void Button::tick() {
    int reading = BUTTON_UP;
    if (this->use_interrupts) {
        reading = this->state_from_interrupt;
    } else {
        reading = digitalRead(this->pin);
        if (this->btn_pulls_to_ground) {
            reading = !reading; // Invert the reading if the button pulls to ground
        }
    }
    
    unsigned long current_time = millis();
    if (reading != this->debounce_state) {
        this->debounce_state = reading;
        this->last_debounce_time = current_time;
    }

    if ((current_time - this->last_debounce_time) > this->debounce_delay_ms) {
        if (reading != this->state) {
            // State has changed (and stabilised)
            this->duration_in_previous_state = current_time - this->time_entered_state;
            this->time_entered_state = current_time;
            this->state = reading;
            this->state_handled = false;

            if (this->state == BUTTON_DOWN) {
                this->time_of_last_up = current_time - this->duration_in_previous_state;
                this->last_press_time = current_time;             
                if (this->onDownCallback) {
                    this->onDownCallback();
                }
            } else {
                if (this->onUpCallback) {
                    this->onUpCallback();
                }
            }
        }
    }

    if (!this->state_handled && this->state == BUTTON_UP) {
        unsigned long time_since_last_up = this->time_of_last_up > 0 ? (current_time - this->time_of_last_up) : ULONG_MAX;
        
        // If the button is up, check if it was long pressed
        if (this->duration_in_previous_state >= this->long_press_time_ms) {
            this->state_handled = true; // Mark the state as handled
            if (this->onLongPressedCallback) {
                this->onLongPressedCallback(this->duration_in_previous_state);
            }
        } else if (time_since_last_up < this->double_press_time_ms && this->duration_in_previous_state > 0) { 
            // Check if it was double pressed (second press within double_press_time_ms)
            this->state_handled = true; // Mark the state as handled
            this->time_of_last_up = 0;
            if (this->onDoublePressedCallback) {
                this->onDoublePressedCallback(time_since_last_up);
            }
        } else if (this->duration_in_previous_state > 0 && time_since_last_up >= this->double_press_time_ms) { 
            // Check if it was a single press (no second press within double_press_time_ms)
            this->state_handled = true; // Mark the state as handled
            if (this->onPressedCallback) {
                this->onPressedCallback(this->duration_in_previous_state);
            }
        }
    }
}

void Button::onPressed(std::function<void(long)> callback) {
    this->onPressedCallback = callback;
}

void Button::onDoublePressed(std::function<void(long)> callback) {
    this->onDoublePressedCallback = callback;
}

void Button::onLongPressed(std::function<void(long)> callback) {
    this->onLongPressedCallback = callback;
}

void Button::onDown(std::function<void()> callback) {
    this->onDownCallback = callback;
}

void Button::onUp(std::function<void()> callback) {
    this->onUpCallback = callback;
}

unsigned long Button::getTimeInLastState() const {
    return this->duration_in_previous_state;
}

unsigned long Button::getTimeInCurrentState() const {
    return millis() - this->time_entered_state;
}

bool Button::isDown() const {
    return this->state == BUTTON_DOWN;
}

bool Button::isUp() const {
    return this->state == BUTTON_UP;
}
