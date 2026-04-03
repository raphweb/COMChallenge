#include <ProbeNodes.hpp>
#include <cstdint>

MatrixPanel_I2S_DMA* ProbeNodes::dma_display;
TwoWire* ProbeNodes::i2c;
ThumbStick* ProbeNodes::st2;
ACAN_ESP32* ProbeNodes::can;
std::map<uint8_t, uint8_t*>* ProbeNodes::baseBoards;
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
        for(auto entry : *baseBoards) {
            uint8_t lastSetting = 0;
            for(int8_t i = 3; i >= 0; i--) {
                lastSetting |= ((1UL << i) | (1UL << (i+4)));
                settingsToGoThrough.emplace_back(std::pair<uint8_t, uint8_t>{entry.first, lastSetting});
            }
        }
        for(auto entry : *baseBoards) {
            uint8_t lastSetting = UINT8_MAX;
            for(int8_t i = 3; i >= 0; i--) {
                lastSetting &= ~((1UL << i) | (1UL << (i+4)));
                settingsToGoThrough.emplace_back(std::pair<uint8_t, uint8_t>{entry.first, lastSetting});
            }
        }
        log_i("Finished scanning for I2C devices.");
    } else {
        uint64_t curTime = millis();
        if (nextSwitchTime < curTime && curSettingNumber < settingsToGoThrough.size()) {
            //log_d("Switching %s ESP32_%d and ESP32_%d at address %d", switchOn ? "on" : "off", i + 1, 8 - i, entry.first);
            auto entry = settingsToGoThrough[curSettingNumber];
            i2c->beginTransmission(entry.first);
            i2c->write(entry.second);
            i2c->endTransmission();
            log_d("Writing %d to the bus.", entry.second);
            baseBoards->emplace(entry.first, &entry.second);
            curSettingNumber++;
            curSettingNumber %= settingsToGoThrough.size();
            nextSwitchTime = curTime + 2100;
        }
    }
    return nullptr;
}