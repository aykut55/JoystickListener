#pragma once
#include <string>
#include <Windows.h>

inline std::string GetKeyName(int vkCode, bool shiftPressed = false) {
    switch (vkCode) {
        case VK_SPACE: return "Space";
        case VK_RETURN: return "Enter";
        case VK_BACK: return "Backspace";
        case VK_INSERT: return "Insert";
        case VK_DELETE: return "Delete";
        case VK_HOME: return "Home";
        case VK_END: return "End";
        case VK_PRIOR: return "PageUp";
        case VK_NEXT: return "PageDown";
        case VK_LEFT: return "ArrowLeft";
        case VK_RIGHT: return "ArrowRight";
        case VK_UP: return "ArrowUp";
        case VK_DOWN: return "ArrowDown";
        case VK_LCONTROL: return "LeftCtrl";
        case VK_RCONTROL: return "RightCtrl";
        case VK_LSHIFT: return "LeftShift";
        case VK_RSHIFT: return "RightShift";
        case VK_LMENU: return "LeftAlt";
        case VK_RMENU: return "RightAlt";
        case VK_OEM_COMMA: return shiftPressed ? "<" : ",";
        case VK_OEM_PERIOD: return shiftPressed ? ">" : ".";
        case VK_OEM_1: return shiftPressed ? ":" : ";";
        case VK_OEM_2: return shiftPressed ? "?" : "/";
        case VK_OEM_3: return shiftPressed ? "~" : "`";
        case VK_OEM_4: return shiftPressed ? "{" : "[";
        case VK_OEM_5: return shiftPressed ? "|" : "\\";
        case VK_OEM_6: return shiftPressed ? "}" : "]";
        case VK_OEM_7: return shiftPressed ? "\"": "'";
        case VK_LBUTTON: return "Mouse Left";
        case VK_RBUTTON: return "Mouse Right";
        case VK_MBUTTON: return "Mouse Middle";
        case VK_XBUTTON1: return "Mouse X1";
        case VK_XBUTTON2: return "Mouse X2";
    }

    UINT scanCode = MapVirtualKey(vkCode, MAPVK_VK_TO_VSC) << 16;
    char name[128] = {0};
    if (GetKeyNameTextA(scanCode, name, sizeof(name)) > 0)
        return name;

    return "Unknown";
}