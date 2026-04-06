#pragma once

#ifndef MENU_TREE_NODE_HPP
#define MENU_TREE_NODE_HPP

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <Wire.h>
#include <ThumbStick.hpp>
#include <ACAN_ESP32.h>
#include <cstdint>
#include <MatrixPanel.hpp>
#include <map>

struct MenuNode {
    const String* const label;
    uint8_t lastKidIndex;
    std::vector<struct MenuNode*>* const kids;
    MenuNode* const parent;
    std::function<void(struct GlobalState*)> setup;
    std::function<MenuNode*()> loop;

    // for root node of menu
    MenuNode(std::function<void(GlobalState*)> _setupFunction, std::function<MenuNode*()> _loopFunction):
        label(new String("main")),
        lastKidIndex(0),
        kids(new std::vector<MenuNode*>),
        parent(nullptr),
        setup(_setupFunction),
        loop(_loopFunction) {}
    
    MenuNode(const char* _label, MenuNode* _parent, std::function<void(GlobalState*)> _setupFunction, std::function<MenuNode*()> _loopFunction):
        label(new String(_label)),
        lastKidIndex(0),
        kids(nullptr),
        parent(_parent),
        setup(_setupFunction),
        loop(_loopFunction) {}

    MenuNode(const char* _label, MenuNode* _parent):
        label(new String(_label)),
        lastKidIndex(0),
        kids(new std::vector<MenuNode*>),
        parent(_parent),
        setup(nullptr),
        loop(nullptr) {}
    
    boolean hasSetupFunction() {
        return true && setup;
    }

    boolean hasLoopFunction() {
        return true && loop;
    }
};

struct GlobalState {
    std::map<uint8_t, uint8_t*>* const baseBoards;
    MatrixPanel_I2S_DMA* const dma_display;
    TwoWire* const i2c;
    ThumbStick* const st1;
    ThumbStick* const st2;
    ACAN_ESP32* const can;
    struct MenuNode* curMenu;
};

template<typename T>
void setup(GlobalState* context) {
    T::setup(context);
}

template<typename T>
MenuNode* loop() {
    return T::loop();
}

//typedef std::pair<std::function<void(GlobalState*)>, std::function<MenuNode*()>> SetupLoopPair;
//typedef std::map<const MenuNode*, SetupLoopPair> ActionRegistry;

class MainMenu {
    friend void setup<MainMenu>(GlobalState* context);
    friend MenuNode* loop<MainMenu>();

    private: static MatrixPanel_I2S_DMA* dma_display;
    private: static ThumbStick* st2;
    private: static MenuNode* curMenu;
    private: static uint8_t menuSel;

    public: static GFX_Layer background_layer;
    public: static GFX_Layer foreground_layer;
    public: static GFX_LayerCompositor compositor;

    protected: static void setup(GlobalState* context);
    protected: static MenuNode* loop();

    private: static void drawInfoPanelMainMenu(uint16_t xOffset, int16_t scroll);
};

#endif