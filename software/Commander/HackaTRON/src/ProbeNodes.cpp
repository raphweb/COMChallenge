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
}

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
                log_i("Found I2C device with address %d", curAddr);
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
    } else if (curSettingNumber < settingsToGoThrough.size()) {
        uint64_t curTime = millis();
        if (nextSwitchTime < curTime) {
            if ((curSettingNumber+1)%8 == 0) {
                switchBBBitMask(0);
            }
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
                    hwId2Node->emplace(hwId, bbNode);
                    nextSwitchTime = curTime + 1000;
                    switchBBBitMask(0);
                    log_i("Received a new join from hwId %#08x.", hwId);
                }
            }
        }
    } else {
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
        log_i("Going back to main menu.");
        return returnMenu;
    }
    return nullptr;
}

void ProbeNodes::switchAllNodes(TwoWire& i2c, boolean on) {
    for(uint8_t i = 0; i < 8; i++) {
        uint8_t curAddr = 0x20 + i;
        i2c.beginTransmission(curAddr);
        if (i2c.endTransmission() == 0) {
            log_i("Found I2C device with address %d", curAddr);
            log_i("Switching all off.");
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