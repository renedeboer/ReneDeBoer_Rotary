#include "rotary.h"

namespace renedeboer {

Rotary* Rotary::_instance = nullptr;

Rotary::Rotary(uint8_t clkPin, uint8_t dtPin, uint8_t swPin, unsigned long longPressTime)
    : _clkPin(clkPin), _dtPin(dtPin), _swPin(swPin), _longPressTime(longPressTime) {}

void Rotary::begin() {
    _instance = this;

    pinMode(_clkPin, INPUT_PULLUP);
    pinMode(_dtPin, INPUT_PULLUP);
    if (!(_swPin == A0)) pinMode(_swPin, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(_clkPin), isrHandlerStatic, RISING);
}

bool Rotary::pressed() {
     
    return _pressed;
}

void Rotary::update() {
    
    // Only read analog pin every 20ms to avoid blocking
    static unsigned long lastAnalogRead = 0;
    unsigned long now = millis();
    if (_swPin == A0 && now - lastAnalogRead >= 20) { 
        lastAnalogRead = now;
        int swVal = analogRead(_swPin);
        _pressed = swVal < 100;  // cache pressed state
    } else if (_swPin != A0) {
        _pressed = digitalRead(_swPin) == LOW;  // fast, non-blocking digital read
    }

    if (_buttonState == IDLE && _pressed) {
        _buttonState = PRESSED;
        _pressStart = millis();
    }
    else if (_buttonState == PRESSED) {
        if (!_pressed) {
            // short press
            if (millis() - _pressStart < _longPressTime) {
                if (_onShortPress) _onShortPress();
            }
            _buttonState = IDLE;
        } else if (millis() - _pressStart >= _longPressTime) {
            // long press
            if (_onLongPress) _onLongPress();
            _buttonState = LONG;
        }
    }
    else if (_buttonState == LONG && !_pressed) {
        _buttonState = IDLE; // release after long press
    }
}


int Rotary::getPosition() const {
    noInterrupts();
    int v = _encoderPos;
    interrupts();
    return v;
}

void Rotary::setOnShortPress(Callback cb) { _onShortPress = cb; }
void Rotary::setOnLongPress(Callback cb) { _onLongPress = cb; }

// ----------------- ISR -----------------
void IRAM_ATTR Rotary::isrHandlerStatic() {
    if (_instance) _instance->handleISR();
}

void IRAM_ATTR Rotary::handleISR() {
    static unsigned long lastMicros = 0;
    unsigned long now = micros();
    if (now - lastMicros < 500) return; // 500us debounce
    lastMicros = now;

    bool clk = digitalRead(_clkPin);
    bool dt  = digitalRead(_dtPin);

    if (clk != dt) {
        _encoderPos--;
    } else {
        _encoderPos++;
    }
    _turned = true;
}


} // namespace renedeboer
