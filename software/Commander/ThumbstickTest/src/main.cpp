#include <Arduino.h>
#include <ESP32AnalogRead.h>

#define LOWER_THRESHHOLD 1000
#define UPPER_THRESHHOLD 3000

ESP32AnalogRead s1v(16), s1h(17), s2v(8), s2h(3);

enum E_Direction { C, N, NE, E, SE, S, SW, W, NW };
const char* DirNames[9] { "·", "🡹", "🡽", "🡺", "🡾", "🡻", "🡿", "🡸", "🡼"};

const E_Direction calculateDirection(const uint16_t h, const uint16_t v) {
  if (h < LOWER_THRESHHOLD) {
    if (v < LOWER_THRESHHOLD) return SW;
    else if (v > UPPER_THRESHHOLD) return NW;
    else return W;
  } else if (h > UPPER_THRESHHOLD) {
    if (v < LOWER_THRESHHOLD) return SE;
    else if (v > UPPER_THRESHHOLD) return NE;
    else return E;
  } else {
    if (v < LOWER_THRESHHOLD) return S;
    else if (v > UPPER_THRESHHOLD) return N;
    else return C;
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
    DirNames[calculateDirection(x1, y1)], (btn1 ? ' ' : 'X'), DirNames[calculateDirection(x2, y2)], (btn2 ? ' ' : 'X'));
  delay(100);
}