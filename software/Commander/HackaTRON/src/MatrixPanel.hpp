#pragma once

#ifndef MATRIX_PANEL_HPP
#define MATRIX_PANEL_HPP

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <GFX_Layer.hpp>

#include <cstdint>
#include <utility>
#include <vector>

#define PANEL_RES_X 64 // Number of pixels wide of each INDIVIDUAL panel module. 
#define PANEL_RES_Y 32 // Number of pixels tall of each INDIVIDUAL panel module.
#define PANEL_CHAIN 4  // Total number of panels chained one to another

// if defined, game stats will be displayed on a 128x32 matrix
// otherwise a display of 64x64 is assumed
#define PANEL_GAME_STATS_WIDE
// if the panel height is 32, we assume that the whole display is rotated,
// except for the lower part of the game grid; if the whole display is rotated
// the game stats panel is actually left of the game grid
#if defined(PANEL_RES_Y) && PANEL_RES_Y == 32
  #define PANELOFFSET 0
  #if defined(PANEL_GAME_STATS_WIDE)
    #define GRIDOFFSET 128
  #endif
#else
  #define PANELOFFSET 64
  #define GRIDOFFSET 0
#endif

#define COLOR565(r, g, b) ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)

const uint16_t c_black = COLOR565(  0,  0,  0);
const uint16_t c_white = COLOR565(255,255,255);
const uint16_t c_red   = COLOR565(255, 31, 15);

inline MatrixPanel_I2S_DMA* setupMatrixPanel() {
    // LED Matrix configuration
    HUB75_I2S_CFG mxconfig(
        PANEL_RES_X, PANEL_RES_Y, PANEL_CHAIN,
        {GPIO_NUM_41, GPIO_NUM_42, GPIO_NUM_40, GPIO_NUM_39, GPIO_NUM_38, GPIO_NUM_37,
        GPIO_NUM_35, GPIO_NUM_36, GPIO_NUM_48, GPIO_NUM_45, GPIO_NUM_21,
        GPIO_NUM_47, GPIO_NUM_2, GPIO_NUM_1}
    );
    mxconfig.clkphase = false;
    mxconfig.latch_blanking = 4;
    // Display Setup
    MatrixPanel_I2S_DMA* dma_display = new MatrixPanel_I2S_DMA(mxconfig);
#if defined(PANEL_RES_Y) && PANEL_RES_Y == 32
    dma_display->setRotation(2);
#endif
    dma_display->begin();
    dma_display->setBrightness8(127); //0-255
    dma_display->clearScreen();
    dma_display->setTextSize(1);
    dma_display->setTextWrap(false);
    return dma_display;
};

class AnimationHandler {
    public: static void run();
    public: static void clearAnimation();
    public: static void addAnimationStep(uint32_t durationMillis, std::function<void()> funcToCall);
    public: static void activate();
    public: static void deactivate();

    private: static boolean active;
    private: static std::vector<std::pair<uint32_t, std::function<void()>>> animationSteps;
    private: static uint8_t curAnimIndex;
    private: static uint64_t nextStep;
};

#endif