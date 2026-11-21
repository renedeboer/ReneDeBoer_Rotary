#include "rotary.h"

namespace renedeboer {

Rotary* Rotary::_instance = nullptr;

Rotary::Rotary(uint8_t clkPin, uint8_t dtPin, uint8_t swPin, unsigned long longPressTime,
               uint8_t stepsPerDetent)
    : _clkPin(clkPin), _dtPin(dtPin), _swPin(swPin), _longPressTime(longPressTime),
      _stepsPerDetent(stepsPerDetent ? stepsPerDetent : 1) {}

void Rotary::begin() {
    _instance = this;

    pinMode(_clkPin, INPUT_PULLUP);
    pinMode(_dtPin, INPUT_PULLUP);
    
    _swIsAnalog = isAnalogCapable(_swPin);
    _pressed = false;
    if (!_swIsAnalog) pinMode(_swPin, INPUT_PULLUP);

    // Only CLK needs an interrupt (DT may be on a pin without one, like ESP8266 GPIO16)
    _lastClk = digitalRead(_clkPin);
    attachInterrupt(digitalPinToInterrupt(_clkPin), isrHandlerStatic, CHANGE);
}

bool Rotary::pressed() {
     
    return _pressed;
}

void Rotary::update() {
    
    // Only read analog pin every 20ms to avoid blocking
    static unsigned long lastAnalogRead = 0;
    unsigned long now = millis();
    if (_swIsAnalog) {
        if (now - lastAnalogRead >= 20) { 
            lastAnalogRead = now;
            int swVal = analogRead(_swPin);
            _pressed = swVal < 100;  // analog threshold
        }
    } else {
        _pressed = digitalRead(_swPin) == LOW;
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

// Runs on every CLK edge. While CLK changes, DT is stable (quadrature), so
// CLK != DT is one direction and CLK == DT the other, on both edges.
// Contact bounce on CLK gives alternating edges that cancel each other out,
// so no time-based debounce is needed (that one used to swallow real edges).
void IRAM_ATTR Rotary::handleISR() {
    bool clk = digitalRead(_clkPin);
    if (clk == _lastClk) return;  // edges merged by interrupt latency
    _lastClk = clk;

    bool dt = digitalRead(_dtPin);
    _step += (clk != dt) ? -1 : 1;

    if (_step >= (int8_t)_stepsPerDetent) {
        _encoderPos++;
        _step = 0;
        _turned = true;
    } else if (_step <= -(int8_t)_stepsPerDetent) {
        _encoderPos--;
        _step = 0;
        _turned = true;
    }
}


} // namespace renedeboer
