#include "rotary.h"

namespace renedeboer {

Rotary* Rotary::_instance = nullptr;

Rotary::Rotary(uint8_t clkPin, uint8_t dtPin, uint8_t swPin, unsigned long longPressTime)
    : _clkPin(clkPin), _dtPin(dtPin), _swPin(swPin), _longPressTime(longPressTime)
{
    _swIsAnalog = (swPin == A0);
}

void Rotary::begin() {
    _instance = this;

    pinMode(_clkPin, INPUT_PULLUP);
    pinMode(_dtPin, INPUT_PULLUP);
    if (!_swIsAnalog) pinMode(_swPin, INPUT_PULLUP);

    attachInterrupt(digitalPinToInterrupt(_clkPin), isrHandlerStatic, RISING);
}

void Rotary::update() {
    int swVal = _swIsAnalog ? analogRead(_swPin) : digitalRead(_swPin);

    bool pressed = _swIsAnalog ? (swVal < 100) : (swVal == LOW);

    if (_buttonState == IDLE && pressed) {
        _buttonState = PRESSED;
        _pressStart = millis();
    }
    else if (_buttonState == PRESSED) {
        if (!pressed) {
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
    else if (_buttonState == LONG && !pressed) {
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

    // read CLK rising edge
    if (!digitalRead(_clkPin)) return;

    delayMicroseconds(40); // small RC delay
    bool dt1 = digitalRead(_dtPin);
    delayMicroseconds(30);
    bool dt2 = digitalRead(_dtPin);

    if (dt1 == dt2) {
        if (dt1 != digitalRead(_clkPin))
            _encoderPos--; // CW
        else
            _encoderPos++; // CCW
        _turned = true;
    }
}

} // namespace renedeboer
