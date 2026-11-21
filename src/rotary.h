#pragma once
#include <Arduino.h>


// ---------------------------------------------------------------------------
// Platform Compatibility Helpers
// ---------------------------------------------------------------------------

// Define IRAM_ATTR safely for non-ESP architectures
#ifndef IRAM_ATTR
#define IRAM_ATTR
#endif

// ---------------------------------------------------------------------------
// Define A0 if missing (some cores don't define it)
// ---------------------------------------------------------------------------
#if !defined(A0)
  #if defined(ESP32)
    #define A0 36
  #elif defined(ESP8266)
    #define A0 A0  // placeholder
  #elif defined(__AVR__)
    #define A0 14   // Arduino Uno/Nano default
  #elif defined(ARDUINO_ARCH_RP2040)
    #define A0 26
  #elif defined(ARDUINO_ARCH_SAMD)
    #define A0 PIN_A0
  #else
    #define A0 0    // fallback
  #endif
#endif

// ---------------------------------------------------------------------------
// Inline function to check if a pin is analog-capable
// ---------------------------------------------------------------------------
static inline constexpr bool isAnalogCapable(uint8_t pin) {
#if defined(ESP32)
    // ADC1 pins 32–39
    return (pin >= 32 && pin <= 39);

#elif defined(ESP8266)
    return (pin == A0);

#elif defined(__AVR__)
    // Handle Uno/Nano (A0–A5) and Mega (A0–A15)
    #ifdef A15
      return (pin >= A0 && pin <= A15);
    #else
      return (pin >= A0 && pin <= A5);
    #endif

#elif defined(ARDUINO_ARCH_RP2040)
    // GPIO26–GPIO29 are analog inputs
    return (pin >= 26 && pin <= 29);

#elif defined(ARDUINO_ARCH_SAMD) || defined(ARDUINO_ARCH_SAM)
    // Typical SAMD boards: A0–A7
    return (pin >= A0 && pin <= (A0 + 7));

#else
    return false;
#endif
}

namespace renedeboer {

class Rotary {
public:
    // Callbacks
    using Callback = void (*)();

    // stepsPerDetent: CLK edges per click. 2 = full quadrature cycle per click (most
    // KY-040 style encoders), 1 = half cycle per click (CLK changes level every click).
    Rotary(uint8_t clkPin, uint8_t dtPin, uint8_t swPin, unsigned long longPressTime = 1000,
           uint8_t stepsPerDetent = 2);

    void begin();
    void update();               // call in loop to handle button
    int getPosition() const;
	bool pressed();
    void setOnShortPress(Callback cb);
    void setOnLongPress(Callback cb);

private:
    uint8_t _clkPin;
    uint8_t _dtPin;
    uint8_t _swPin;

    volatile int _encoderPos = 0;
    volatile bool _turned = false;
    volatile int8_t _step = 0;      // CLK edges since the last counted click
    volatile bool _lastClk = true;
    uint8_t _stepsPerDetent;

    // button
    enum ButtonState { IDLE, PRESSED, LONG };
    ButtonState _buttonState = IDLE;
    unsigned long _pressStart = 0;
    unsigned long _longPressTime;

    Callback _onShortPress = nullptr;
    Callback _onLongPress = nullptr;

    bool _swIsAnalog = false;
    bool  _pressed = false;
    // ISR helpers
    static Rotary* _instance;
    static void IRAM_ATTR isrHandlerStatic();
    void IRAM_ATTR handleISR();
};

} // namespace renedeboer
