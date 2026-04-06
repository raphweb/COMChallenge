#include <MatrixPanel.hpp>
#include <MainMenu.hpp>
#include <cstdint>


MatrixPanel_I2S_DMA* MainMenu::dma_display;
ThumbStick* MainMenu::st2;
MenuNode* MainMenu::curMenu;
uint8_t MainMenu::menuSel = 0;

// Layer objects with improved memory management
GFX_Layer MainMenu::background_layer(PANEL_RES_X*2, PANEL_RES_Y,
    [](int16_t x, int16_t y, uint8_t r, uint8_t g, uint8_t b) {
        dma_display->drawPixelRGB888(x, y, r, g, b);
    });

GFX_Layer MainMenu::foreground_layer(PANEL_RES_X*2, PANEL_RES_Y,
    [](int16_t x, int16_t y, uint8_t r, uint8_t g, uint8_t b) {
        dma_display->drawPixelRGB888(x, y, r, g, b);
    });

// Compositor with advanced blending
GFX_LayerCompositor MainMenu::compositor([](int16_t x, int16_t y, uint8_t r, uint8_t g, uint8_t b) {
        dma_display->drawPixelRGB888(x, y, r, g, b);
    });

void MainMenu::setup(GlobalState* context) {
    dma_display = context->dma_display;
    st2 = context->st2;
    curMenu = context->curMenu;
    menuSel = curMenu->lastKidIndex;
    // Verify layer initialization
    if (!background_layer.isInitialized()) {
        log_e("Background layer failed to initialize!");
        return;
    }
    if (!foreground_layer.isInitialized()) {
        log_e("Foreground layer failed to initialize!");
        return;
    }
    drawInfoPanelMainMenu(XOFFSET, 0);
}
    
MenuNode* MainMenu::loop() {
    if (st2->dirEvents()) {
        ThumbStick::E_Direction moveDir = st2->getNextDirEvent();
        if (moveDir == ThumbStick::N || moveDir == ThumbStick::S) {
            for(uint16_t sc = 0; sc < 8; sc++) {
                drawInfoPanelMainMenu(XOFFSET, moveDir == ThumbStick::N ? sc : -sc);
                delay(15);
            }
            menuSel += curMenu->kids->size();
            moveDir == ThumbStick::N ? menuSel-- : menuSel++;
            menuSel %= curMenu->kids->size();
            drawInfoPanelMainMenu(XOFFSET, 0);
        } else if (moveDir == ThumbStick::E) {
            curMenu->lastKidIndex = menuSel;
            curMenu = curMenu->kids->at(menuSel);
            if (!curMenu->kids || curMenu->kids->empty()) {
                log_i("Going to menu %s.", curMenu->label->c_str());
            } else {
                for(uint16_t sc = 0; sc < PANEL_RES_X; sc++) {
                    if (sc % 2) {
                        dma_display->drawFastVLine(XOFFSET + PANEL_RES_X*2 - sc/2 - 1, 0, 8, c_black);
                        dma_display->drawFastVLine(XOFFSET + PANEL_RES_X*2 - sc/2 - 1, 16, 16, c_black);
                    }
                    dma_display->fillRect(XOFFSET + sc*2, 0, 2, 8, c_black);
                    dma_display->fillRect(XOFFSET + sc*2, 16, 2, 16, c_black);
                    delay(8);
                }
                log_i("Going to submenu %s.", curMenu->label->c_str());
            }
            st2->clearEventQueues();
            return curMenu;
        } else if (moveDir == ThumbStick::W) {
            if (curMenu->parent) {
                for(uint16_t sc = 0; sc < PANEL_RES_X-4; sc++) {
                    dma_display->fillRect(XOFFSET + PANEL_RES_X*2 - sc*2 - 1, 0, 2, PANEL_RES_Y, c_black);
                    delay(8);
                }
                curMenu->lastKidIndex = menuSel;
                curMenu = curMenu->parent;
                log_i("Going back to menu %s.", curMenu->label->c_str());
                st2->clearEventQueues();
                return curMenu;
            }
        }
    }
    if ((!curMenu->kids || curMenu->kids->empty()) && curMenu->parent) {
        return curMenu->parent;
    }
    return nullptr;
}

void MainMenu::drawInfoPanelMainMenu(int16_t xOffset, int16_t yOffset, int16_t width, int16_t height) {
    if (yOffset == 0 && xOffset == XOFFSET) {
        for(uint8_t i = 0; i < 8; i++) {
            background_layer.fastFillRect(xOffset, 7-i, width, 1, CRGB(140-i*12,140-i*12,140-i*12));
        }
        background_layer.fastFillRect(xOffset, 8, width, 8, CRGB::Red);
        for(uint8_t i = 0; i < 16; i++) {
            background_layer.fastFillRect(xOffset, 16+i, width, 1, CRGB(140-i*6,140-i*6,140-i*6));
        }
    }
    foreground_layer.fastFillRect(xOffset, 0, width, height, c_black);
    foreground_layer.transparency_colour = c_black;
    foreground_layer.setTextColor(c_white);
    uint8_t countToRoot = 0;
    MenuNode* cur = curMenu->parent;
    while (cur) {
        cur = cur->parent;
        countToRoot++;
    }
    for(uint8_t i = 0; i < 6; i++) {
        uint8_t entry = (curMenu->kids->size() + menuSel + i - 2) % curMenu->kids->size();
        foreground_layer.setCursor(xOffset + (countToRoot<<3), 8*i - 8 + yOffset);
        MenuNode* mn = curMenu->kids->at(entry);
        foreground_layer.print(mn->label->c_str());
        if (mn->kids && !mn->kids->empty()) {
            foreground_layer.fastFillRect(PANEL_RES_X*2 - 6, 8*i - 8 + yOffset, 5, 8, c_black);
            foreground_layer.setCursor(PANEL_RES_X*2 - 6, 8*i - 8 + yOffset);
            foreground_layer.print('>');
        }
    }
    if (countToRoot > 0) {
        foreground_layer.fastFillRect(0, 8, 6*countToRoot, 8, c_black);
        foreground_layer.setCursor(0, 8);
        for(uint8_t i = 0; i < countToRoot; i++) {
            foreground_layer.print('<');
        }
    }
    compositor.Siloette(background_layer, foreground_layer);
}