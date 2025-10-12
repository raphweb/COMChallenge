#include <Arduino.h>

/*#include <ESP32-TWAI-CAN.hpp>

#define CAN_TX 5
#define CAN_RX 7

void setup() {
    if(ESP32Can.begin(TWAI_SPEED_500KBPS, CAN_TX, CAN_RX, 4, 4)) {
        log_d("CAN bus started!");
    } else {
        log_e("CAN bus failed!");
    }
}

void loop() {

}*/

// ESP32 C3 Super Mini on-board LED (works with inverted logic)
const int ledPin = 8;

void setup() {
  pinMode(ledPin, OUTPUT);
}

void loop() {
  digitalWrite(ledPin, HIGH);
  log_d("LED OFF");
  delay(1000);
  digitalWrite(ledPin, LOW); 
  log_d("LED ON");
  delay(1000);
}
