#pragma once

#ifndef PROBE_NODES_HPP
#define PROBE_NODES_HPP

#include <MainMenu.hpp>

class ProbeNodes {
    friend void setup<ProbeNodes>(GlobalState* context);
    friend MenuNode* loop<ProbeNodes>();

    private: static MatrixPanel_I2S_DMA* dma_display;
    private: static TwoWire* i2c;
    private: static ThumbStick* st2;
    private: static ACAN_ESP32* can;
    private: static std::map<uint8_t, uint8_t*>* baseBoards;
    private: static std::map<uint32_t, uint8_t>* hwId2Node;
    private: static std::vector<std::pair<uint8_t, uint8_t>> settingsToGoThrough;
    private: static uint8_t curSettingNumber;
    private: static uint64_t nextSwitchTime;
    private: static MenuNode* returnMenu;

    public: static void switchAllNodes(TwoWire& i2c, boolean on);

    protected: static void setup(GlobalState* context);
    protected: static MenuNode* loop();

    private: static void switchBB();
    private: static void switchBBBitMask(uint8_t bitMask);
};

#endif