# ReneDeBoer_Rotary

Rotary encoder library for Arduino/ESP8266 with support for:

* Single-pin interrupt for rotary signals (CLK pin)
* Analog or digital button input with external pullup support
* Long press and short press detection with callbacks
* Smooth handling of fast and slow rotations

This library is designed for **EC11**, **HW-040**, and similar encoders, tested on ESP8266 (Wemos D1).

---

## Features

* Increment/decrement rotary position using a single interrupt pin
* Optional button handling via digital or analog input
* Configurable long press time
* Simple API for reading position and detecting changes

---

## Installation

1. Clone or download the repository.
2. Place the folder `ReneDeBoer_Rotary` in your Arduino libraries folder:

```
~/Documents/Arduino/libraries/
```

3. Restart Arduino IDE.
4. Include the library in your sketch:

```cpp
#include <rotary.h>
```

---

## Example: BasicRotary

The `BasicRotary` example shows how to:

* Initialize the rotary encoder
* Read rotation changes
* Detect short and long button presses

```cpp
#include <ReneDeBoer_Rotary/rotary.h>

// Pins: CLK, DT, Button (can be analog)
renedeboer::Rotary rotary(D6, D0, A0);

void setup() {
  Serial.begin(115200);
  rotary.begin();
  
  rotary.onShortPress([]() {
    Serial.println("Short press detected!");
  });

  rotary.onLongPress([]() {
    Serial.println("Long press detected!");
  });
}

void loop() {
  rotary.update();

  if (rotary.hasChanged()) {
    Serial.print("Position: ");
    Serial.println(rotary.getPosition());
    rotary.resetChanged();
  }
}
```

**Notes:**

* `rotary.update()` must be called regularly in `loop()`.
* Short and long press callbacks are optional.
* Analog button pin requires a **10k external pull-up resistor**.

---

## API

| Function                 | Description                                                  |
| ------------------------ | ------------------------------------------------------------ |
| `begin()`                | Initialize pins and attach interrupt                         |
| `update()`               | Read rotary and button state, must be called in loop         |
| `getPosition()`          | Get current rotary position                                  |
| `hasChanged()`           | Returns true if position changed since last `resetChanged()` |
| `resetChanged()`         | Reset change flag after handling                             |
| `onShortPress(callback)` | Assign callback for short press                              |
| `onLongPress(callback)`  | Assign callback for long press                               |
| `setLongPressTime(ms)`   | Set threshold for long press detection                       |

---

## Supported Hardware

* Rotary encoders with two pins for rotation (CLK + DT)
* Optional push button on analog or digital pin
* EC11 and HW-040 tested

---

## License

MIT License
