#include "esp32-hal-gpio.h"
#include <Arduino.h>
#include <ESP32AnalogRead.h>
#include <cstdint>

#define LOWER_THRESHHOLD 1000
#define UPPER_THRESHHOLD 3000

ESP32AnalogRead s1v(16), s1h(17), s2v(8), s2h(3);

const char* calculateDirection(uint16_t h, uint16_t v) {
  if (h < LOWER_THRESHHOLD) {
    if (v < LOWER_THRESHHOLD) {
      return "⇙";
    } else if (v > UPPER_THRESHHOLD) {
      return "⇖";
    } else {
      return "⇐";
    }
  } else if (h > UPPER_THRESHHOLD) {
    if (v < LOWER_THRESHHOLD) {
      return "⇘";
    } else if (v > UPPER_THRESHHOLD) {
      return "⇗";
    } else {
      return "⇒";
    }
  } else {
    if (v < LOWER_THRESHHOLD) {
      return "⇓";
    } else if (v > UPPER_THRESHHOLD) {
      return "⇑";
    } else {
      return "·";
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(18, INPUT_PULLUP);
  pinMode( 9, INPUT_PULLUP);
}

void loop() {
  uint16_t y1 = (4095-s1v.readRaw()), y2 = s2v.readRaw();
  uint16_t x1 = (4095-s1h.readRaw()), x2 = s2h.readRaw();
  boolean btn1 = digitalRead(18), btn2 = digitalRead(9);
  Serial.printf("S1(%s|%c)   S2(%s|%c)\r",
    calculateDirection(x1, y1), (btn1 ? ' ' : 'X'), calculateDirection(x2, y2), (btn2 ? ' ' : 'X'));
  delay(100);
}