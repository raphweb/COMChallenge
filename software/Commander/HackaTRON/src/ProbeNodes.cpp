#include <MatrixPanel.hpp>
#include <ProbeNodes.hpp>
#include <ACAN_ESP32.h>
#include <cstdint>

MatrixPanel_I2S_DMA* ProbeNodes::dma_display;
TwoWire* ProbeNodes::i2c;
ThumbStick* ProbeNodes::st2;
ACAN_ESP32* ProbeNodes::can;
std::map<uint8_t, uint8_t*>* ProbeNodes::baseBoards;
std::map<uint32_t, uint8_t>* ProbeNodes::hwId2Node;
std::vector<std::pair<uint8_t, uint8_t>> ProbeNodes::settingsToGoThrough;
uint8_t ProbeNodes::curSettingNumber;
uint64_t ProbeNodes::nextSwitchTime;
MenuNode* ProbeNodes::returnMenu;

void ProbeNodes::setup(GlobalState* context) {
    returnMenu = context->curMenu->parent;
    dma_display = context->dma_display;
    i2c = context->i2c;
    st2 = context->st2;
    can = context->can;
    baseBoards = context->baseBoards;
    baseBoards->clear();
    hwId2Node = context->hwId2Node;
    hwId2Node->clear();
    settingsToGoThrough.clear();
    curSettingNumber = 0;
    nextSwitchTime = 0;
    dma_display->clearScreen();
    AnimationHandler::clearAnimation();
    AnimationHandler::addAnimationStep(500, []() { drawProbingNode(c_white); });
    AnimationHandler::addAnimationStep(500, []() { drawProbingNode(c_red); });
}

boolean onTime = false;

MenuNode* ProbeNodes::loop() {
    if (st2->dirEvents()) {
        ThumbStick::E_Direction moveDir = st2->getNextDirEvent();
        if (moveDir == ThumbStick::W) {
            st2->clearEventQueues();
            log_i("Going back to main menu.");
            return returnMenu;
        }
    }
    if (baseBoards->empty()) {
        log_i("Scanning for I2C devices...");
        for(uint8_t i = 0; i < 8; i++) {
            uint8_t curAddr = 0x20 + i;
            i2c->beginTransmission(curAddr);
            if (i2c->endTransmission() == 0) {
                log_i("Found I2C device with address 0x%02X.", curAddr);
                baseBoards->emplace(curAddr, new uint8_t(0));
                log_i("Switching all off.");
                i2c->beginTransmission(curAddr);
                i2c->write(*baseBoards->at(curAddr));
                i2c->endTransmission();
            }
        }
        for(auto bb: *baseBoards) {
            for(uint8_t i = 0; i < 4; i++) {
                settingsToGoThrough.emplace_back(std::pair<uint8_t, uint8_t>{bb.first, i});
            }
            for(uint8_t i = 7; i >=4; i--) {
                settingsToGoThrough.emplace_back(std::pair<uint8_t, uint8_t>{bb.first, i});
            }
        }
        log_i("Finished scanning for I2C devices.");
        switchBB();
        dma_display->setTextColor(COLOR565(0,144,240));
        dma_display->setCursor(GRIDOFFSET + PANEL_RES_X, 0);
        dma_display->printf("%d", settingsToGoThrough.size());
        dma_display->setTextColor(COLOR565(248,120,0));
        dma_display->setCursor(GRIDOFFSET + PANEL_RES_X + 22, 0);
        dma_display->printf("addres-");
        dma_display->setCursor(GRIDOFFSET + PANEL_RES_X, 8);
        dma_display->print("sable");
        dma_display->setCursor(dma_display->getCursorX()+5, 8);
        dma_display->print("nodes");
        dma_display->setCursor(GRIDOFFSET + PANEL_RES_X, 16);
        dma_display->print("found,");
        dma_display->setCursor(GRIDOFFSET + PANEL_RES_X, 24);
        dma_display->print("probing...");
        AnimationHandler::activate();
    } else if (curSettingNumber < settingsToGoThrough.size()) {
        uint64_t curTime = millis();
        if (nextSwitchTime < curTime) {
            if ((curSettingNumber+1)%8 == 0) {
                switchBBBitMask(0);
            }
            if (onTime) {
                drawProbingNode(COLOR565(104,0,224));
            } else {
                drawProbingNode(c_red);
            }
            onTime = false;
            curSettingNumber++;
            switchBB();
        } else { // keep waiting for JOIN frame
            CANMessage frame;
            while (ACAN_ESP32::can.receive(frame)) {
                if ((frame.id & 0x7FF) == 0x100 && frame.len == 4) { // JOIN frame
                    auto entry = settingsToGoThrough[curSettingNumber];
                    uint8_t bbId = entry.first & 0x07; // only use the lower three bits
                    uint8_t node = entry.second & 0x07; // there are only 8 different nodes per base board
                    uint8_t bbNode = (bbId << 4) | node;
                    uint32_t hwId = frame.data32[0];
                    if (!hwId2Node->count(hwId)) {
                        hwId2Node->emplace(hwId, bbNode);
                    } else {
                        log_e("A node with hwId 0x%08X has already registered! Aborting...");
                        switchBBBitMask(0);
                        log_i("Going back to main menu");
                        return returnMenu;
                    }
                    nextSwitchTime = curTime + 1000;
                    switchBBBitMask(0);
                    log_i("Received a new join from hwId 0x%08X.", hwId);
                    onTime = true;
                }
            }
        }
    } else if (curSettingNumber == settingsToGoThrough.size()) {
        AnimationHandler::deactivate();
        AnimationHandler::clearAnimation();
        // probing finished, switch on all nodes that have answered (i.e. sent a JOIN frame)
        for(auto entry: *hwId2Node) {
            uint8_t bbNode = entry.second;
            uint8_t bbId = ((bbNode & 0x70) >> 4) | 0x20;
            uint8_t* nodeBitMask = baseBoards->at(bbId);
            *nodeBitMask |= (1 << (bbNode & 0x07));
        }
        for(auto bb: *baseBoards) {
            i2c->beginTransmission(bb.first);
            i2c->write(*bb.second);
            i2c->endTransmission();
        }
        dma_display->fillRect(GRIDOFFSET + PANEL_RES_X, 24, PANEL_RES_X, 8, c_black);
        dma_display->setTextColor(COLOR565(224,248,0));
        dma_display->setCursor(GRIDOFFSET + PANEL_RES_X + 41, 16);
        dma_display->printf("%d", hwId2Node->size());
        dma_display->setTextColor(COLOR565(248,120,0));
        dma_display->setCursor(GRIDOFFSET + PANEL_RES_X, 24);
        dma_display->print("registered!");
        nextSwitchTime = millis() + 1000;
        curSettingNumber++;
    } else {
        uint64_t curTime = millis();
        if (nextSwitchTime < curTime) {
            log_i("Going back to main menu.");
            return returnMenu;
        }
    }
    AnimationHandler::run();
    return nullptr;
}

void ProbeNodes::switchAllNodes(TwoWire& i2c, boolean on) {
    for(uint8_t i = 0; i < 8; i++) {
        uint8_t curAddr = 0x20 + i;
        i2c.beginTransmission(curAddr);
        if (i2c.endTransmission() == 0) {
            log_i("Found I2C device with address 0x%02X.", curAddr);
            log_i("Switching all %s.", on ? "on" : "off");
            i2c.beginTransmission(curAddr);
            i2c.write(on ? 0xFF : 0);
            i2c.endTransmission();
        }
    }
}

void ProbeNodes::switchBB() {
    auto entry = settingsToGoThrough[curSettingNumber];
    switchBBBitMask(1 << entry.second);
    CANMessage frame;
    while (ACAN_ESP32::can.receive(frame)) {};
    nextSwitchTime = millis() + 10000;
}

void ProbeNodes::switchBBBitMask(uint8_t bitMask) {
    auto entry = settingsToGoThrough[curSettingNumber];
    i2c->beginTransmission(entry.first);
    i2c->write(bitMask);
    i2c->endTransmission();
    log_d("Writing %d to the bus.", bitMask);
    baseBoards->emplace(entry.first, &bitMask);
}

void ProbeNodes::drawProbingNode(uint16_t color) {
    auto entry = settingsToGoThrough[curSettingNumber];
    uint8_t halfNodeWidth = 128 / settingsToGoThrough.size();
    uint8_t xOffset = PANELOFFSET + (128 - halfNodeWidth * settingsToGoThrough.size()) / 2 +
        (entry.first & 7) * 8 * halfNodeWidth +
        (entry.second < 4 ? 3 - entry.second : 7 - entry.second) * halfNodeWidth * 2 + halfNodeWidth / 2;
    uint8_t yOffset = entry.second < 4 ? 22 : 4;
    uint8_t barWidth = 64 / settingsToGoThrough.size();
    uint8_t gridXOffset = (64 - barWidth * settingsToGoThrough.size()) / 2;
    dma_display->fillRect(xOffset, yOffset, halfNodeWidth, 6, color);
    dma_display->fillRect(GRIDOFFSET + PANEL_RES_X - gridXOffset - (curSettingNumber+1)*barWidth, 12, barWidth, 8, color);
}