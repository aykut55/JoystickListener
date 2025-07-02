#pragma once

enum class KeyState {
    Down,
    Hold,
    Up
};

struct KeyEvent {
    int vkCode;
    KeyState state;
    bool shift = false;
    bool ctrl = false;
    bool alt = false;
};