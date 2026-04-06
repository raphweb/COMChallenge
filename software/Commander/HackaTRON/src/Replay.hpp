#pragma once

#ifndef REPLAY_HPP
#define REPLAY_HPP

#include <MainMenu.hpp>
#include <ACAN_ESP32_CANMessage.h>
#include <ESP32TimerInterrupt.hpp>
#include <FS.h>
#include <LittleFS.h>

#define MAX_FRAMES_PER_READ 32
typedef struct frame {
    uint8_t data[10];
} frame;
typedef struct gameFrame {
    uint16_t id = 0;
    uint8_t length = 0;
    uint8_t *data = nullptr;
} gameFrame;

class Replay {
    friend void setup<Replay>(GlobalState* context);
    friend MenuNode* loop<Replay>();

    private: static String* gameToReplay;
    private: static ThumbStick* st2;
    private: static MenuNode* returnMenu;

    private: static frame buffer[MAX_FRAMES_PER_READ*sizeof(frame)];
    private: static uint16_t currentFrameIndex;
    private: static uint16_t lastFrame;
    private: static CANMessage* currentGF;
    private: static File currentGameDir;
    private: static File currentGameFile;
    private: static uint32_t iCtr;
    private: static uint32_t speedFactor;

    private: static ESP32Timer* iTimer;

    protected: static void setup(GlobalState* context);
    protected: static MenuNode* loop();

    private: static int parsePacket();
    private: static void listDir(fs::FS &fs, const char *dirname, uint8_t levels);
    private: static boolean onTimer(void *timerNo);
};

#if defined(CREATE_EXAMPLE_RECORDING)

struct GameRecord {
  const String gameName;
  const uint8_t *content;
  const size_t size;
};

#include <Games/game0.h>
#include <Games/game1.h>
#include <Games/game2.h>
#include <Games/game3.h>
#include <Games/game4.h>
#include <Games/game5.h>
#include <Games/game6.h>
#include <Games/game7.h>
#include <Games/game8.h>
#include <Games/game9.h>
#include <Games/game10.h>
#include <Games/game11.h>
#include <Games/game12.h>
#include <Games/game13.h>
#include <Games/game14.h>
#include <Games/game15draw.h>
#include <Games/game16best.h>

GameRecord hackaTron2025Games[] = {
  {"game00", (const uint8_t *)&Game0Buffer, GAME0_SIZE},
  {"game01", (const uint8_t *)&Game1Buffer, GAME1_SIZE},
  {"game02", (const uint8_t *)&Game2Buffer, GAME2_SIZE},
  {"game03", (const uint8_t *)&Game3Buffer, GAME3_SIZE},
  {"game04", (const uint8_t *)&Game4Buffer, GAME4_SIZE},
  {"game05", (const uint8_t *)&Game5Buffer, GAME5_SIZE},
  {"game06", (const uint8_t *)&Game6Buffer, GAME6_SIZE},
  {"game07", (const uint8_t *)&Game7Buffer, GAME7_SIZE},
  {"game08", (const uint8_t *)&Game8Buffer, GAME8_SIZE},
  {"game09", (const uint8_t *)&Game9Buffer, GAME9_SIZE},
  {"game10", (const uint8_t *)&Game10Buffer, GAME10_SIZE},
  {"game11", (const uint8_t *)&Game11Buffer, GAME11_SIZE},
  {"game12", (const uint8_t *)&Game12Buffer, GAME12_SIZE},
  {"game13", (const uint8_t *)&Game13Buffer, GAME13_SIZE},
  {"game14", (const uint8_t *)&Game14Buffer, GAME14_SIZE},
  {"game15draw", (const uint8_t *)&Game15drawBuffer, GAME15DRAW_SIZE},
  {"game16best", (const uint8_t *)&Game16bestBuffer, GAME16BEST_SIZE}
};

void deleteFiles(const String &path) {
  log_i("Delete: %s", path.c_str());
  if (!LittleFS.remove(path)) {
    File root = LittleFS.open(path);
    while (String filename = root.getNextFileName()) {
      LittleFS.remove(filename);
      LittleFS.rmdir(path);
      if (filename.length() < 1) break;
    }
  }
}

void createExampleRecording() {
  const String dir = "/HackaTron2025";
  if (!LittleFS.exists(dir)) {
    if (LittleFS.mkdir(dir)) {
      log_i("Successfully created directory '%s'.", dir.c_str());
    }
  } else {
    deleteFiles(dir);
    log_i("Removed directory '%s'.", dir.c_str());
    while(1) {};
  }
  for(GameRecord gr:hackaTron2025Games) {
    if (LittleFS.exists(dir + "/" + gr.gameName + ".record")) {
      continue;
    }
    log_i("Writing game '%s/%s.record'.", dir.c_str(), gr.gameName.c_str());
    File file = LittleFS.open(dir + "/" + gr.gameName + ".record", FILE_WRITE);
    file.write(gr.content, gr.size*sizeof(frame));
    file.flush();
    file.close();
    log_i("Successfully written game record.");
  }
  while(1) {} // don't do anything else
}
#endif

#endif