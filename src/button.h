
#include <Arduino.h>
#include <functional>

#define BUTTON_UP 0
#define BUTTON_DOWN 1


class Button {
    private:
        int pin;
        int state;
        unsigned long time_entered_state;
        bool use_interupts;
        bool btn_pulls_to_ground;
        int state_from_interrupt;
        unsigned long debounce_delay_ms;
        unsigned long last_debounce_time;
        int debounce_state;
        unsigned long duration_in_previous_state;
        unsigned long duration_in_this_state_previously;
        unsigned long long_press_time_ms;
        unsigned long double_press_time_ms;
        unsigned long last_press_time;
        bool state_handled;

        void handleOnDownInterrupt();
        void handleOnUpInterrupt();

        std::function<void(long)> onPressedCallback;
        std::function<void(long)> onDoublePressedCallback;
        std::function<void(long)> onLongPressedCallback;
        std::function<void()> onDownCallback;
        std::function<void()> onUpCallback;
        
    public:
        /** 
         * @brief Construct a new Button object
         * @param pin The pin number the button is connected to
         * @param debounce_delay_ms The debounce delay in milliseconds - aka. the period of time that the button state must remain stable for the button state to be considered changed (default is 25ms)
         * @param use_interupts Whether to use interrupts for button state changes - when true, ticks will not read the pin state, so you can safely call tick more frequently (default is true) [Set to false when the pin does not support interrupts]
         * @param btn_pulls_to_ground Whether the button pulls to ground when pressed - aka. When the button is pressed the button reads LOW (default is true)
         * @param long_press_time_ms The time in milliseconds that you must depress the button for, for it to be considered a long press (default is 800ms)
         * @param double_press_time_ms The time in milliseconds within which you must re-press the button to trigger a double press (default is 300ms)
         * @note The button state is considered pressed when the pin reads LOW if btn_pulls_to_ground is true, otherwise it is considered pressed when the pin reads HIGH.
         */
        Button(int pin, unsigned long debounce_delay_ms = 25, bool use_interupts = true, bool btn_pulls_to_ground = true, unsigned long long_press_time_ms = 800, unsigned long double_press_time_ms = 300);

        /** 
         * @brief Tick the button - you must call this function regularly to update the button state
         * @note If use_interupts is true, this function will not read the pin state, so you can safely call tick more frequently, otherwise, you should call this function more frequently than half the debounce_delay_ms milliseconds.
         */
        void tick();

        /**
         * @brief Set the callback function to be called when the button is pressed
         * @param callback The callback function to be called when the button is pressed
         * @note The callback function will be called with the time in milliseconds that the button was held pressed down for
         */
        void onPressed(std::function<void(long)> callback);         // long is the time in ms that the button was pressed down for
        /**
         * @brief Set the callback function to be called when the button is double pressed
         * @param callback The callback function to be called when the button is double pressed
         * @note The callback function will be called with the time in milliseconds between the two presses
         */
        void onDoublePressed(std::function<void(long)> callback);   // long is the time between the two presses in ms
        /**
         * @brief Set the callback function to be called when the button is long pressed
         * @param callback The callback function to be called when the button is long pressed
         * @note The callback function will be called with the time in milliseconds that the button was held pressed down for
         */
        void onLongPressed(std::function<void(long)> callback);     // long is the time in ms that the button was pressed down for  
        /**
         * @brief Set the callback function to be called when the button is pressed down
         * @param callback The callback function to be called when the button is pressed down
         */
        void onDown(std::function<void()> callback);
        /**
         * @brief Set the callback function to be called when the button is released
         * @param callback The callback function to be called when the button is released
         */
        void onUp(std::function<void()> callback);

        /**
         * @brief Returns a boolean indicating whether the button is currently pressed down
         * @return true if the button is pressed down, false otherwise
         */
        bool isDown();
        /**
         * @brief Returns a boolean indicating whether the button is currently released
         * @return true if the button is released, false otherwise
         */
        bool isUp();
        /**
         * @brief Returns the time in milliseconds that the button was in it's previous state for
         * @return The time in milliseconds that the button has was in the previous state for
         */
        unsigned long getTimeInLastState();

        /**
         * @brief Returns the time in milliseconds that the button has been in it's current state for
         * @return The time in milliseconds that the button has been in the current state for
         */
        unsigned long getTimeInCurrentState();
};;