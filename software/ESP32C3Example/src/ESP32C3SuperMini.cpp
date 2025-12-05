#include <Arduino.h>
#include <ACAN_ESP32.h>

const uint32_t hardware_ID = ESP.getEfuseMac() >> 16;

// ESP32 C3 Super Mini on-board LED (works with inverted logic)
const int ledPin = 8;

void setup() {
  pinMode(ledPin, OUTPUT);
  ACAN_ESP32_Settings settings(500UL * 1000UL);
  settings.mRequestedCANMode = ACAN_ESP32_Settings::NormalMode;
  settings.mRxPin = GPIO_NUM_6; // Optional, default Tx pin is GPIO_NUM_4
  settings.mTxPin = GPIO_NUM_5; // Optional, default Rx pin is GPIO_NUM_5
  const ACAN_ESP32_Filter filter = ACAN_ESP32_Filter::dualStandardFilter(
    ACAN_ESP32_Filter::data, 0x123, 0x110,
    ACAN_ESP32_Filter::data, 0x456, 0x022
  );
  const uint32_t errorCode = ACAN_ESP32::can.begin(settings, filter);
  if (errorCode) {
    log_e("Configuration error %08x", errorCode);
  }
}

void loop() {
  digitalWrite(ledPin, HIGH);
  log_d("LED OFF");
  delay(1000);
  digitalWrite(ledPin, LOW); 
  log_d("LED ON");
  uint8_t macAddr[6];
  esp_efuse_mac_get_default(macAddr);
  log_d("CHIP MAC: %02x:%02x:%02x:%02x:%02x:%02x", MAC2STR(macAddr));
  delay(1000);
  CANMessage frame;
  frame.id = 0x100;
  frame.len = 4;
  frame.data32[0] = hardware_ID;
  ACAN_ESP32::can.tryToSend(frame);
  log_d("JOIN packet sent (Hardware ID: %u)", hardware_ID);
}
