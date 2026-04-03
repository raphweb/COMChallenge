#pragma once

#ifndef THUMBSTICK_HPP
#define THUMBSTICK_HPP

#include <ESP32AnalogRead.h>
#include <cstdint>
#include <MD_CirQueue.h>

#define QUEUE_BUFFER_SIZE 8

class ThumbStick {

public:

    enum E_Direction { C, N, NE, E, SE, S, SW, W, NW, NODIR };
    enum E_Button { PRESSED, RELEASED, NOBUT };

    static const char* dirName(E_Direction dir) {
        static const char* dirNames[10] { "·", "🡹", "🡽", "🡺", "🡾", "🡻", "🡿", "🡸", "🡼", " " };
        return dirNames[dir];
    }

    ThumbStick(const int& xAxisPin = -1, const int& yAxisPin = -1, const int& pushButtonPin = -1, const boolean _flipX = false, const boolean _flipY = false, const boolean _ignoreDiagonals = true) :
            dirEventQueue(QUEUE_BUFFER_SIZE, sizeof(E_Direction)),
            butEventQueue(QUEUE_BUFFER_SIZE, sizeof(E_Button)),
            flipX(_flipX),
            flipY(_flipY),
            ignoreDiagonals(_ignoreDiagonals) {
        if (xAxisPin > -1) {
            xAxis = new ESP32AnalogRead(xAxisPin);
        }
        if (yAxisPin > -1) {
            yAxis = new ESP32AnalogRead(yAxisPin);
        }
        if (pushButtonPin > -1) {
            pinMode(pushButtonPin, INPUT_PULLUP);
            buttonPin = pushButtonPin;
        }
        dirEventQueue.begin();
        butEventQueue.begin();
    }

    ~ThumbStick() {
        clearEventQueues();
        if (xAxis) {
            delete xAxis;
        }
        if (yAxis) {
            delete yAxis;
        }
    }

    const boolean dirEvents() {
        return !dirEventQueue.isEmpty();
    }

    const boolean buttonEvents() {
        return !butEventQueue.isEmpty();
    }

    const E_Direction getNextDirEvent() {
        E_Direction ret = NODIR;
        dirEventQueue.pop((uint8_t *)&ret);
        return ret;
    }

    const E_Button getNextButtonEvent() {
        E_Button ret = NOBUT;
        butEventQueue.pop((uint8_t *)&ret);
        return ret;
    }

    void sampleState() {
        uint16_t xSample = flipX ? 4095 - xAxis->readRaw() : xAxis->readRaw();
        uint16_t ySample = flipY ? 4095 - yAxis->readRaw() : yAxis->readRaw();
        E_Direction sampledDir = calculateDirection(xSample, ySample);
        if (lastDirSample != sampledDir) {
            dirEventQueue.push((uint8_t *)&sampledDir);
            lastDirSample = sampledDir;
            log_d("Raw X: %d; Y: %d", xSample, ySample);
        }
        boolean butSample = digitalRead(buttonPin);
        E_Button sampledBut = butSample ? RELEASED : PRESSED;
        if (lastButSample != sampledBut) {
            butEventQueue.push((uint8_t *)&sampledBut);
            lastButSample = sampledBut;
        }
    }

    const E_Direction getCurrentDir() {
        return lastDirSample;
    }

    const E_Button getCurrentBut() {
        return lastButSample;
    }

    void clearEventQueues() {
        dirEventQueue.clear();
        butEventQueue.clear();
    }

    void calibrate() {
        uint16_t xSample = flipX ? 4095 - xAxis->readRaw() : xAxis->readRaw();
        uint16_t ySample = flipY ? 4095 - yAxis->readRaw() : yAxis->readRaw();
        uint16_t avg = (xSample + ySample)/2;
        lowerThreshhold = avg - (avg/3);
        upperThreshhold = avg + (avg/3);
        log_i("Calibrated lower threshhold: %4d, upper threshhold: %4d.", lowerThreshhold, upperThreshhold);
    }

private:
    ESP32AnalogRead* xAxis;
    ESP32AnalogRead* yAxis;
    uint16_t upperThreshhold = 3000;
    uint16_t lowerThreshhold = 1000;
    uint8_t buttonPin = 255;
    boolean flipX = false, flipY = false, ignoreDiagonals = true;
    E_Direction lastDirSample = C;
    E_Button lastButSample = RELEASED;
    MD_CirQueue dirEventQueue;
    MD_CirQueue butEventQueue;

    const E_Direction calculateDirection(const uint16_t& x, const uint16_t& y) {
        if (x < lowerThreshhold) {
            if (y < lowerThreshhold) {
                if (ignoreDiagonals) {
                    if (x/8 > y/8) {
                        return W;
                    } else {
                        return S;
                    }
                } else {
                    return SW;
                }
            } else if (y > upperThreshhold) {
                if (ignoreDiagonals) {
                    if (x/8 > (4095-y)/8) {
                        return W;
                    } else {
                        return N;
                    }
                } else {
                    return NW;
                }
            } else {
                return W;
            }
        } else if (x > upperThreshhold) {
            if (y < lowerThreshhold) {
                if (ignoreDiagonals) {
                    if ((4095-x)/8 > y/8) {
                        return E;
                    } else {
                        return S;
                    }
                } else {
                    return SE;
                }
            } else if (y > upperThreshhold) {
                if (ignoreDiagonals) {
                    if (x/8 > y/8) {
                        return E;
                    } else {
                        return N;
                    }
                } else {
                    return NE;
                }
            } else {
                return E;
            }
        } else {
            if (y < lowerThreshhold) return S;
            else if (y > upperThreshhold) return N;
            else return C;
        }
    }

    const E_Direction calcYDir(const uint16_t& y) {
        if (y < lowerThreshhold) return S;
        else if (y > upperThreshhold) return N;
        else return C;
    }

    const E_Direction calcXDir(const uint16_t& x) {
        if (x < lowerThreshhold) return W;
        else if (x > upperThreshhold) return E;
        else return C;
    }

};

#endif