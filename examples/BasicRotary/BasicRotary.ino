#include <Arduino.h>
#include "rotary.h"

#define CLK D6
#define DT  D0
#define SW  A0

renedeboer::Rotary rotary(CLK, DT, SW);

void shortPress() { Serial.println("Short press!"); }
void longPress()  { Serial.println("Long press!"); }

void setup() {
  Serial.begin(115200);
  rotary.begin();
  rotary.setOnShortPress(shortPress);
  rotary.setOnLongPress(longPress);
}

void loop() {
  rotary.update();

  static int lastPos = -999;
  int pos = rotary.getPosition();
  if (pos != lastPos) {
      Serial.print("Position: ");
      Serial.println(pos);
      lastPos = pos;
  }
}
