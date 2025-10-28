#pragma once
#include <Arduino.h>

namespace renedeboer {

class Rotary {
public:
    // Callbacks
    using Callback = void (*)();

    Rotary(uint8_t clkPin, uint8_t dtPin, uint8_t swPin, unsigned long longPressTime = 1000);

    void begin();
    void update();               // call in loop to handle button
    int getPosition() const;

    void setOnShortPress(Callback cb);
    void setOnLongPress(Callback cb);

private:
    uint8_t _clkPin;
    uint8_t _dtPin;
    uint8_t _swPin;

    volatile int _encoderPos = 0;
    volatile bool _turned = false;

    // button
    enum ButtonState { IDLE, PRESSED, LONG };
    ButtonState _buttonState = IDLE;
    unsigned long _pressStart = 0;
    unsigned long _longPressTime;

    Callback _onShortPress = nullptr;
    Callback _onLongPress = nullptr;

    bool _swIsAnalog = false;

    // ISR helpers
    static Rotary* _instance;
    static void IRAM_ATTR isrHandlerStatic();
    void IRAM_ATTR handleISR();
};

} // namespace renedeboer
