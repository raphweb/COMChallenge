#include <Arduino.h>
#include <ESP32AnalogRead.h>

ESP32AnalogRead s1v(16), s1h(17);

void setup() {
  Serial.begin(115200);
}

void loop() {
  uint16_t y = s1v.readRaw() >> 8;
  uint16_t x = s1h.readRaw() >> 8;
  Serial.printf("Value: (x: %2d, y: %2d)\n", x, y);
  delay(100);
}