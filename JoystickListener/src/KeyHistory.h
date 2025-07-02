#pragma once
#include <chrono>
#include "KeyEvent.h"

struct KeyHistory {
    int pressCount = 0;
    int currentHoldCount = 0;
    bool isPressed = false;
    bool wasPressed = false;
    bool isReleased = false;
    bool wasReleased = false;
    KeyState lastState = KeyState::Up;
    std::chrono::steady_clock::time_point lastPressedTime;
    std::chrono::steady_clock::time_point lastReleasedTime;
};