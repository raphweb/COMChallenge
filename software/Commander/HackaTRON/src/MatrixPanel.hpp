#pragma once

#ifndef MATRIX_PANEL_HPP
#define MATRIX_PANEL_HPP

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <GFX_Layer.hpp>

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
#define XOFFSET 0
#else
#define XOFFSET 64
#endif

#define COLOR565(r, g, b) ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)

const uint16_t c_black = COLOR565(  0,  0,  0);
const uint16_t c_white = COLOR565(255,255,255);
const uint16_t c_red   = COLOR565(255, 31, 15);

#endif