#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <Wire.h>
#include <ThumbStick.hpp>
#include <ACAN_ESP32.h>
#include <ACAN_ESP32_Settings.h>
#include <LittleFS.h>

#if defined(CORE_DEBUG_LEVEL) && (CORE_DEBUG_LEVEL > 0)
#define SETUP_SERIAL_CONSOLE \
  esp_rom_install_uart_printf();\
  delay(200);\
  log_printf(ARDUHAL_LOG_COLOR_I "[%6u][ ][%s:%u] %s(): Started serial log." ARDUHAL_LOG_RESET_COLOR "\r\n", (unsigned long) (esp_timer_get_time() / 1000ULL), pathToFileName(__FILE__), __LINE__, __FUNCTION__);
#else
#define SETUP_SERIAL_CONSOLE
#endif

//#define CREATE_EXAMPLE_RECORDING

#include <MatrixPanel.hpp>
#include <MainMenu.hpp>
#include <ProbeNodes.hpp>
#include <Replay.hpp>

GlobalState* globStat;
MenuNode* menu;

TwoWire i2c(0);
ThumbStick* st1;
ThumbStick* st2;

void fillMenu() {
  menu->kids->emplace_back(new MenuNode("Probe connected nodes", menu, &setup<ProbeNodes>, &loop<ProbeNodes>));
  menu->kids->emplace_back(new MenuNode("Host single game", menu, nullptr, nullptr));
  menu->kids->emplace_back(new MenuNode("Host a tournament", menu, nullptr, nullptr));
  Replay::fillReplayMenu(menu);
  menu->kids->emplace_back(new MenuNode("Observe and display", menu, nullptr, nullptr));
  MenuNode* advancedOptions = new MenuNode("Advanced options", menu);
  advancedOptions->kids->emplace_back(new MenuNode("Disable all nodes", advancedOptions, [](GlobalState* context) { ProbeNodes::switchAllNodes(*context->i2c, false); }, nullptr));
  advancedOptions->kids->emplace_back(new MenuNode("Enable all nodes", advancedOptions, [](GlobalState* context) { ProbeNodes::switchAllNodes(*context->i2c, true); }, nullptr));
  advancedOptions->kids->emplace_back(new MenuNode("Test upper stick", advancedOptions, nullptr, nullptr));
  advancedOptions->kids->emplace_back(new MenuNode("Test lower stick", advancedOptions, nullptr, nullptr));
  menu->kids->emplace_back(advancedOptions);
}

void setup() {
  SETUP_SERIAL_CONSOLE
  // start LittleFS
  if(!LittleFS.begin(true)) {
    log_e("LittleFS Mount Failed");
    while (1) {}
  }
#ifdef CREATE_EXAMPLE_RECORDING
  createExampleRecording();
#endif
  // init and fill menu entries
  menu = new MenuNode(&setup<MainMenu>, &loop<MainMenu>);
  fillMenu();
  MatrixPanel_I2S_DMA* dma_display = setupMatrixPanel();
  // calibraty thumbsticks
  st1 = new ThumbStick(GPIO_NUM_17,GPIO_NUM_16,GPIO_NUM_18,true,true);
  st1->calibrate();
  st2 = new ThumbStick(GPIO_NUM_3,GPIO_NUM_8,GPIO_NUM_9);
  st2->calibrate();
  // timer setup to periodically read thumbstick states
  ESP32Timer iTimer1(1);
  iTimer1.setInterval(64000UL, [](void* timerNo) -> boolean {
    st1->sampleState();
    st2->sampleState();
    return true;
  });
  // I2C bus setup
  i2c.begin(GPIO_NUM_43, GPIO_NUM_44, 10UL * 1000UL); // 10 kHz
  // CAN bus setup
  ACAN_ESP32_Settings settings(500UL * 1000UL); // 500 kBaud
  settings.mTxPin = GPIO_NUM_15;
  settings.mRxPin = GPIO_NUM_7;
  const ACAN_ESP32_Filter filter = ACAN_ESP32_Filter::acceptStandardFrames();
  uint32_t errorCode = ACAN_ESP32::can.begin(settings, filter);
  if (errorCode == 0) {
    log_i("Bit rate prescaler: %d;", settings.mBitRatePrescaler);
    log_i("Time segment 1:     %d;", settings.mTimeSegment1);
    log_i("Time segment 2:     %d;", settings.mTimeSegment2);
    log_i("RJW:                %d;", settings.mRJW);
    log_i("Triple sampling:    %s;", settings.mTripleSampling ? "yes" : "no");
    uint32_t actBR = settings.actualBitRate();
    if (actBR >= 1000) {
      if (actBR % 1000) {
        log_i("Actual bit rate:    %d.%03d kBit/s;", actBR/1000, actBR%1000);
      } else {
        log_i("Actual bit rate:    %d kBit/s;", actBR/1000);
      }
    } else {
      log_i("Actual bit rate:    %d Bit/s;", actBR);
    }
    log_i("Exact bit rate?     %s;", settings.exactBitRate() ? "yes" : "no");
    log_i("Sample point:       %d%%;", settings.samplePointFromBitStart());
    log_i("Configuration OK :-)");
  } else {
    log_e("Configuration error %#08x!", errorCode);
  }
  // create global state
  globStat = new GlobalState{new std::map<uint8_t, uint8_t*>, new std::map<uint32_t, uint8_t>,
      dma_display, &i2c, st1, st2, &ACAN_ESP32::can, menu};
  if (globStat->curMenu && globStat->curMenu->hasSetupFunction()) {
    globStat->curMenu->setup(globStat);
  }
}

void loop() {
  if (globStat->curMenu) {
    MenuNode* loopRet = globStat->curMenu->hasLoopFunction() ? globStat->curMenu->loop() : menu->loop();
    if (loopRet) {
      if (loopRet->hasSetupFunction()) {
        // curMenu changed
        globStat->curMenu = loopRet;
        loopRet->setup(globStat);
      } else if (loopRet->kids && !loopRet->kids->empty()) {
        globStat->curMenu = loopRet;
        menu->setup(globStat);
      } else {
        menu->setup(globStat);
      }
    }
  }
}
