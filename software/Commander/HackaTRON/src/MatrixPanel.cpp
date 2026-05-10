#include <MatrixPanel.hpp>

boolean AnimationHandler::active = false;
std::vector<std::pair<uint32_t, std::function<void()>>> AnimationHandler::animationSteps;
uint8_t AnimationHandler::curAnimIndex;
uint64_t AnimationHandler::nextStep;

void AnimationHandler::run() {
    if (!active) {
        return;
    }
    uint64_t curTime = millis();
    if (nextStep < curTime) {
        auto entry = animationSteps.at(curAnimIndex);
        entry.second();
        nextStep = curTime + entry.first;
        curAnimIndex++;
        if (curAnimIndex >= animationSteps.size()) {
            curAnimIndex = 0;
        }
    }
}

void AnimationHandler::clearAnimation() {
    animationSteps.clear();
    curAnimIndex = 0;
}

void AnimationHandler::addAnimationStep(uint32_t durationMillis, std::function<void ()> funcToCall) {
    animationSteps.emplace_back(std::pair<uint32_t, std::function<void()>>{durationMillis, funcToCall});
}

void AnimationHandler::activate() {
    active = true;
}

void AnimationHandler::deactivate() {
    active = false;
}