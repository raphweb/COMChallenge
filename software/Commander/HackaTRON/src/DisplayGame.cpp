#include <DisplayGame.hpp>
#include <cstdint>

MatrixPanel_I2S_DMA* DisplayGame::dma_display = nullptr;
uint16_t DisplayGame::receivedFrames = 0;
PlayerInfo* DisplayGame::players[64] = {nullptr};
GameInfo* DisplayGame::currentGame = nullptr;
uint8_t DisplayGame::gameEvent = 0;
uint8_t DisplayGame::gameEventInfo = 0;

void DisplayGame::init(GlobalState* context) {
    dma_display = context->dma_display;
    if (currentGame) {
        delete currentGame;
    }
    currentGame = new GameInfo();
    drawInfoPanelWaitingForGame(XOFFSET);
}

void DisplayGame::processFrame(CANMessage& frame) {
    if (receivedFrames < 1024) {
        log_d("Received Frame with id 0x%03X.", frame.id);
        receivedFrames++;
    }
    switch (frame.id & 0x7FF) {
        case 0x100: // JOIN - ignore this
            break;
        case 0x110: // PLAYER - get the playerID
            if (frame.len == 5 && !players[frame.data[4]]) {
                uint32_t hwId = frame.data32[0];
                players[frame.data[4]] = new PlayerInfo(frame.data[4], hwId);
            }
            break;
        case 0x500: // RENAME - update a players name
            log_buf_d(frame.data, frame.len);
            if (players[frame.data[0]]) {
                PlayerInfo* player = players[frame.data[0]];
                player->nameLength = frame.data[1];
                player->name->clear();
                uint8_t len2add = min((uint8_t)(frame.len-2), player->nameLength);
                player->name->concat(reinterpret_cast<const char*>(&frame.data[2]), len2add);
                if (player->nameLength <= 6) {
                    gameEvent |= PLAYER_RENAMED;
                    gameEventInfo = player->playerID;
                }
            }
            break;
        case 0x510: // RENAMEFOLLOW - keep updating a players name
            log_buf_d(frame.data, frame.len);
            if (players[frame.data[0]]) {
                PlayerInfo* player = players[frame.data[0]];
                if (player->nameLength > player->name->length()) {
                    uint8_t len2add = min((uint8_t)(frame.len-1), (uint8_t)(player->nameLength - player->name->length()));
                    player->name->concat(reinterpret_cast<const char*>(&frame.data[1]), len2add);
                    if (player->nameLength <= player->name->length()) {
                        gameEvent |= PLAYER_RENAMED;
                        gameEventInfo = player->playerID;
                    }
                }
            }
            break;
        case 0x040: // GAME - prepare for a new game about to start
            for(uint8_t i = 0; i < PLAYERS_IN_GAME; i++)
                if (!players[frame.data[i]]) goto breakLabel;
            for(uint8_t i = 0; i < PLAYERS_IN_GAME; i++)
                currentGame->player[i] = players[frame.data[i]];
            gameEvent |= GAME_REQUESTED;
            break;
        case 0x120: // GAMEACK - ignore this
            break;
        case 0x050: // GAMESTATE - update the positions on the grid
            for(uint8_t i = 0; i < PLAYERS_IN_GAME; i++)
                if (!currentGame->player[i]) goto breakLabel;
            for(uint8_t i = 0; i < PLAYERS_IN_GAME; i++)
                if (currentGame->alive[i]) {
                    if (currentGame->moves[i] > 0) {
                        currentGame->tail[i][currentGame->y[i]] |= (1ULL << currentGame->x[i]);
                        drawGridPixel(currentGame->x[i], currentGame->y[i], trailPlayerColors[i]);
                    }
                    currentGame->x[i] = frame.data[i*2];
                    currentGame->y[i] = 63-frame.data[i*2+1];
                }
            gameEvent |= GAME_STATE_UPDATE;
            break;
        case 0x090: // MOVE - ignore this
            break;
        case 0x080: // DIE - a player just died, remove him and his trail
            for(uint8_t i = 0; i < PLAYERS_IN_GAME; i++)
                if (currentGame->player[i]->playerID == frame.data[0]) {
                    currentGame->alive[i] = false;
                    gameEvent |= (PLAYER_DIED << i);
                }
            break;
        case 0x070: // GAMEFINISH - game has finished, clear the grid
            for(uint8_t i = 0; i < PLAYERS_IN_GAME; i++)
                if (currentGame->player[i]->playerID == frame.data[i*2])
                    currentGame->score[i] = frame.data[i*2+1];
            gameEvent |= GAME_FINISH;
            break;
        case 0x020: // ERROR - ignore this
            break;
        default   : breakLabel: break;
    }
}

void DisplayGame::updatedDisplay() {
    if (gameEvent & PLAYER_RENAMED) {
        // a player was renamed
        drawInfoPanelPlayerJoined(XOFFSET, *players[gameEventInfo]->name);
        gameEvent &= (~PLAYER_RENAMED);
        gameEventInfo = 0;
    }
    if (gameEvent & GAME_REQUESTED) {
        // reset current game settings
        for(uint8_t i = 0; i < PLAYERS_IN_GAME; i++) {
            currentGame->alive[i] = true;
            currentGame->moves[i] = 0;
            currentGame->score[i] = 0;
            currentGame->x[i] = UINT8_MAX;
            currentGame->y[i] = UINT8_MAX;
            for(uint8_t y = 0; y < 64; y++)
                currentGame->tail[i][y] = 0;
        }
        dma_display->clearScreen();
        drawInfoPanelInGame(XOFFSET);
        drawInfoPanelPlayerListInNewGame(XOFFSET);
        gameEvent &= (~GAME_REQUESTED);
    }
    if (gameEvent & GAME_STATE_UPDATE) {
        for(uint8_t i = 0; i < PLAYERS_IN_GAME; i++)
            if (currentGame->alive[i]) {
                currentGame->moves[i]++;
                drawGridPixel(currentGame->x[i], currentGame->y[i], activePlayerColors[i]);
            }
        gameEvent &= (~GAME_STATE_UPDATE);
    }
    for(uint8_t i = 0; i < PLAYERS_IN_GAME; i++)
        if (gameEvent & (PLAYER_DIED << i)) {
            for(uint8_t y = 0; y < 64; y++) {
                for(uint8_t x = 0; x < 64; x++)
                    // remove tail
                    if (currentGame->tail[i][y] & (1ULL << x))
                        drawGridPixel(x, y, c_black);
                currentGame->tail[i][y] = 0;
            }
            drawGridPixel(currentGame->x[i], currentGame->y[i], c_black);
            drawInfoPanelPlayerListInGamePlayer(XOFFSET, i, c_black);
            currentGame->x[i] = UINT8_MAX;
            currentGame->y[i] = UINT8_MAX;
            gameEvent &= (~(PLAYER_DIED << i));
        }
    if (gameEvent & GAME_FINISH) {
        for(uint8_t i = 0; i < PLAYERS_IN_GAME; i++) {
            currentGame->player[i]->score += currentGame->score[i];
        }
        drawInfoPanelGameResults(XOFFSET);
        gameEvent &= (~GAME_FINISH);
    }
}

void DisplayGame::clearDisplay() {
    dma_display->clearScreen();
}

inline void DisplayGame::drawGridPixel(uint8_t x, uint8_t y, uint16_t c) {
#if defined(PANEL_RES_Y) && PANEL_RES_Y == 32
    if (y & 0xE0) {
        // rotate display, add 192 to x and subtract 32 from y
        dma_display->drawPixel(((x&0x3F)^0x3F)|0x80, (y&0x1F)^0x1F, c);
    } else {
        dma_display->drawPixel(x|0xC0, y, c);
    }
#else
    dma_display->drawPixel(x, y, c);
#endif
}


void DisplayGame::drawInfoPanelWaitingForGame(int16_t xOffset) {
  dma_display->setTextColor(c_white);
  dma_display->setCursor(xOffset, 0);
#if defined(PANEL_GAME_STATS_WIDE)
  dma_display->println("Waiting for next Game");
#else
  dma_display->println("Waiting for");
  dma_display->setCursor(xOffset, dma_display->getCursorY());
  dma_display->println("next Game.");
#endif
}

void DisplayGame::drawInfoPanelPlayerJoined(uint16_t xOffset, const String &playerName) {
#if defined(PANEL_GAME_STATS_WIDE)
  dma_display->fillRect(xOffset, 8, 128, 24, c_black);
#else
  dma_display->fillRect(xOffset, 16, 64, 24, c_black);
#endif
  dma_display->setTextColor(c_red);
#if defined(PANEL_GAME_STATS_WIDE)
  dma_display->setCursor(xOffset, 8);
#else
  dma_display->setCursor(xOffset, 16);
  if (playerName.length() > 10) {
    dma_display->println(playerName.substring(0, 10));
    dma_display->setCursor(xOffset, dma_display->getCursorY());
    dma_display->println(playerName.substring(10));
  } else {
#endif
    dma_display->println(playerName);
#if not defined(PANEL_GAME_STATS_WIDE)
  }
#endif
  dma_display->setTextColor(c_white);
  dma_display->setCursor(xOffset, dma_display->getCursorY());
  dma_display->println("joined!");
}

void DisplayGame::drawInfoPanelInGame(int16_t xOffset) {
#if not defined(PANEL_GAME_STATS_WIDE)
  dma_display->setTextColor(c_white);
  dma_display->setCursor(xOffset + 5, 0);
  dma_display->println("Currently");
  dma_display->setCursor(xOffset + 8, dma_display->getCursorY());
  dma_display->println("playing:");
#endif
}

void DisplayGame::drawInfoPanelPlayerListInGamePlayer(int16_t xOffset, uint8_t playerNum, uint16_t color) {
#if defined(PANEL_GAME_STATS_WIDE)
  dma_display->setCursor(xOffset, 8 * playerNum);
#else
  dma_display->setCursor(xOffset, 20 + 10 * playerNum);
#endif
  dma_display->setTextColor(color);
  dma_display->print(*currentGame->player[playerNum]->name);
}

void DisplayGame::drawInfoPanelPlayerListInNewGame(int16_t xOffset) {
  for(uint8_t i = 0; i < PLAYERS_IN_GAME; i++) {
    if (currentGame->player[i]) {
      drawInfoPanelPlayerListInGamePlayer(xOffset, i, activePlayerColorsGBSwitched[i % PLAYERS_IN_GAME]);
    }
  }
}

void DisplayGame::drawInfoPanelGameResults(int16_t xOffset) {
#if defined(PANEL_GAME_STATS_WIDE)
      dma_display->fillRect(xOffset,0,128,32,c_black);
#else
      dma_display->fillRect(xOffset,0,64,64,c_black);
#endif
  dma_display->setCursor(xOffset, 0);
  for(uint8_t i = 0; i < PLAYERS_IN_GAME; i++) {
    dma_display->setTextColor(c_white);
    uint8_t scLen = dma_display->printf("%d ", currentGame->score[i]);
    dma_display->setCursor(dma_display->getCursorX() - 1, dma_display->getCursorY());
    dma_display->setTextColor(activePlayerColorsGBSwitched[i % PLAYERS_IN_GAME]);
    const String* playerName = currentGame->player[i]->name;
#if not defined(PANEL_GAME_STATS_WIDE)
    if (playerName->length() + scLen > 11) {
      dma_display->println(playerName->substring(0, 11 - scLen));
      dma_display->setCursor(xOffset, dma_display->getCursorY());
      dma_display->println(playerName->substring(11 - scLen));
    } else {
#endif
      dma_display->println(*playerName);
#if not defined(PANEL_GAME_STATS_WIDE)
    }
#endif
    dma_display->setCursor(xOffset, dma_display->getCursorY());
  }
}
