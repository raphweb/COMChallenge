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

#endif