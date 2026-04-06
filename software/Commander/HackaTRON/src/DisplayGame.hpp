#pragma once

#ifndef DISPLAY_GAME_HPP
#define DISPLAY_GAME_HPP

#include <MainMenu.hpp>
#include <ACAN_ESP32_CANMessage.h>

struct PlayerInfo {
    const uint8_t playerID;
    const uint32_t hwId;
    uint8_t nameLength = 0;
    String* const name;
    uint16_t score = 0;
    explicit PlayerInfo(uint8_t _playerID, uint32_t _hwId) : playerID(_playerID),
            hwId(_hwId), nameLength(11), name(new String(32, 0)) {
        char stdNameBuf[12];
        sprintf(stdNameBuf, "Player: %03d", playerID);
        name->clear();
        name->concat(stdNameBuf, 11);
    };
};

#define PLAYERS_IN_GAME 4  // number of players playing in one game

const uint16_t activePlayerColors[PLAYERS_IN_GAME] = {
  COLOR565(240,0,120), // pink
  COLOR565(224,248,0), // green
  COLOR565(0,144,240), // cyan
  COLOR565(248,120,0), // orange
};

// for some reason the two matrices for the game stats panel
// do not have RGB colors but RBG
const uint16_t activePlayerColorsGBSwitched[PLAYERS_IN_GAME] = {
  COLOR565(240,120,0), // pink
  COLOR565(104,0,224), // green
  COLOR565(0,248,104), // cyan
  COLOR565(240,0,120), // orange
};

const uint16_t trailPlayerColors[PLAYERS_IN_GAME] = {
  COLOR565(80,0,40), // pink
  COLOR565(72,80,0), // green
  COLOR565(0,48,80), // cyan
  COLOR565(88,40,0), // orange
};

struct GameInfo {
  PlayerInfo *player[PLAYERS_IN_GAME];
  boolean alive[PLAYERS_IN_GAME];
  uint8_t x[PLAYERS_IN_GAME], y[PLAYERS_IN_GAME];
  uint8_t score[PLAYERS_IN_GAME] = {0};
  uint16_t moves[PLAYERS_IN_GAME] = {0};
  uint64_t tail[PLAYERS_IN_GAME][64] = {{0}};
  GameInfo() {
    for(uint8_t i = 0; i < PLAYERS_IN_GAME; i++) {
      player[i] = nullptr;
      alive[i] = true;
      x[i] = UINT8_MAX;
      y[i] = UINT8_MAX;
    }
  }
};

// game event flags relevant for display updates containing the following events starting from LSB:
// PLAYER_RENAMED | GAME_REQUESTED | GAME_STATE_UPDATE | GAME_FINISHED | PLAYER[1]_DIED ...
#define PLAYER_RENAMED    (1U << 0)
#define GAME_REQUESTED    (1U << 1)
#define GAME_STATE_UPDATE (1U << 2)
#define GAME_FINISH       (1U << 3)
#define PLAYER_DIED       (1U << 4)

class DisplayGame {
    private: static MatrixPanel_I2S_DMA* dma_display;
    private: static uint16_t receivedFrames;
    private: static PlayerInfo* players[64];
    private: static GameInfo* currentGame;
    private: static uint8_t gameEvent;
    private: static uint8_t gameEventInfo;

    public: static void init(GlobalState* context);
    public: static void processFrame(CANMessage& frame);
    public: static void updatedDisplay();
    public: static void clearDisplay();

    private: static void drawGridPixel(uint8_t x, uint8_t y, uint16_t c);
    private: static void drawInfoPanelWaitingForGame(int16_t xOffset);
    private: static void drawInfoPanelPlayerJoined(uint16_t xOffset, const String &playerName);
    private: static void drawInfoPanelInGame(int16_t xOffset);
    private: static void drawInfoPanelPlayerListInGamePlayer(int16_t xOffset, uint8_t playerNum, uint16_t color);
    private: static void drawInfoPanelPlayerListInNewGame(int16_t xOffset);
    private: static void drawInfoPanelGameResults(int16_t xOffset);
};

#endif