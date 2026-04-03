#include <Replay.hpp>
#include <DisplayGame.hpp>
#include <cstdint>

std::string Replay::gameToReplay;
ThumbStick* Replay::st2;
MenuNode* Replay::returnMenu;

frame Replay::buffer[MAX_FRAMES_PER_READ*sizeof(frame)] = {0};
uint16_t Replay::currentFrame = 0;
uint16_t Replay::lastFrame = 0;
CANMessage* Replay::currentGF = nullptr;
File Replay::currentGameDir;
File Replay::currentGameFile;
uint32_t Replay::iCtr = 0;
uint32_t Replay::speedFactor = 32;

ESP32Timer* Replay::iTimer;

void Replay::setup(GlobalState* context) {
    returnMenu = context->curMenu->parent;
    gameToReplay = "/" + context->curMenu->label;
    st2 = context->st2;
    currentFrame = 0;
    lastFrame = 0;
    currentGameDir = {};
    currentGameFile = {};
    iTimer = new ESP32Timer(2);
    log_d("Created new replay object.");
    listDir(LittleFS, "/", 1);
    iTimer->setInterval(1500, onTimer);
    iTimer->disableTimer();
    DisplayGame::init(context);
    currentGF = new CANMessage();
}

MenuNode* Replay::loop() {
    if (st2->dirEvents()) {
        ThumbStick::E_Direction moveDir = st2->getNextDirEvent();
        if (moveDir == ThumbStick::W) {
            st2->clearEventQueues();
            iTimer->disableTimer();
            iTimer->detachInterrupt();
            delete currentGF;
            DisplayGame::clearDisplay();
            log_i("Going back to main menu.");
            return returnMenu;
        } else if (moveDir == ThumbStick::N) {
            if (speedFactor < 2) {
                speedFactor = 1;
            } else {
                speedFactor /= 2;
            }
        } else if (moveDir == ThumbStick::S) {
            speedFactor *= 2;
        }
    }
    boolean firstTime = false;
    if ((!currentGameFile || currentGameFile.position() == 0) && lastFrame == 0) {
        // start new file read
        log_d("Loading game '%s'.", gameToReplay.c_str());
        if (!currentGameFile) {
            currentGameFile = LittleFS.open(gameToReplay.c_str());
            if (currentGameFile.isDirectory()) {
                currentGameDir = currentGameFile;
                currentGameFile = currentGameDir.openNextFile();
            }
        }
        size_t len = currentGameFile.size();
        lastFrame = len/sizeof(frame);
        log_i("Loaded game '%s' with size %d and replaying it.", currentGameFile.name(), len);
        firstTime = true;
        currentFrame = 0;
    }
    if (currentGameFile && currentFrame % (MAX_FRAMES_PER_READ/2) == 0 && currentGameFile.position() / sizeof(frame) == currentFrame) {
        iTimer->disableTimer();
        size_t remaining = min(lastFrame*sizeof(frame) - currentGameFile.position(), (MAX_FRAMES_PER_READ/2)*sizeof(frame));
        log_d("Reading next %d bytes.", remaining);
        currentGameFile.read((uint8_t *)&buffer[currentFrame % MAX_FRAMES_PER_READ], remaining);
        if (firstTime) {
            currentFrame = 0;
            parsePacket();
            currentFrame = 1;
        }
        if (!currentGameFile.available()) {
            if (currentGameDir) {
                currentGameFile = currentGameDir.openNextFile();
                if (!currentGameFile) {
                log_d("Starting again in directory.");
                currentGameDir.seekDir(0);
                currentGameFile = currentGameDir.openNextFile();
                }
            } else {
                currentGameFile.seek(0);
            }
            log_d("Closed file.");
        }
        iTimer->enableTimer();
        log_d("Enabling the timer.");
    }
    DisplayGame::updatedDisplay();
    return nullptr;
}

int Replay::parsePacket() {
    log_d("Starting to read at position: %d", currentFrame);
    currentGF->id = buffer[currentFrame%MAX_FRAMES_PER_READ].data[0];
    currentGF->id |= ((uint16_t)buffer[currentFrame%MAX_FRAMES_PER_READ].data[1] & 0x7) << 8;
    currentGF->len = (buffer[currentFrame%MAX_FRAMES_PER_READ].data[1] & 0xF0) >> 4;
    for(uint8_t i = 0; i < 8; i++)
        currentGF->data[i] = buffer[currentFrame%MAX_FRAMES_PER_READ].data[2+i];
    log_buf_d(currentGF.data, 8);
    return 1;
}

void Replay::listDir(fs::FS &fs, const char * dirname, uint8_t levels) {
    log_i("Listing directory: %s\r", dirname);
    File root = fs.open(dirname);
    if(!root) {
        log_e("- failed to open directory");
        return;
    }
    if(!root.isDirectory()) {
        log_i(" - not a directory");
        return;
    }
    File file = root.openNextFile();
    while(file) {
        if(file.isDirectory()) {
            log_i("  DIR : %s", file.name());
            if(levels) {
                listDir(fs, (String("/") + String(file.name())).c_str(), levels -1);
            }
        } else {
            log_i("  FILE: %s\tSIZE: %d", file.name(), file.size());
        }
        file = root.openNextFile();
    }
}

bool IRAM_ATTR Replay::onTimer(void *timerNo) {
    if (!currentGF) {
        return true;
    }
    // called every 10ms
    if (currentGF->id == 0x050 && iCtr < speedFactor ||   // wait 32*1.5ms = 48ms before sending next game state (frame id 0x050)
        currentFrame > lastFrame && iCtr < (speedFactor << 7)) {  // wait 4096*1.5ms = ~6s before starting next game
        iCtr++;
    } else {
        if (currentGF->id == 0x050 || currentFrame > lastFrame) {
            // reset counter if we send next game state or start next game
            iCtr = 0;
        }
        if (currentFrame > lastFrame) {
            // start next game
            iTimer->disableTimer();
            currentFrame = 0;
            lastFrame = 0;
            return true;
        }
        // "send" next game state (i.e. call the callback function to process frames)
        DisplayGame::processFrame(*currentGF);
        parsePacket(); // decode next packet from the record file
        currentFrame++;
    }
    return true;
}