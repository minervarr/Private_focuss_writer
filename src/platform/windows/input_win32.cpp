#include "input_win32.h"
#include "utils/logger.h"

namespace phantom {

InputWin32::InputWin32() {
    LOG_TRACE(LogCategory::PLATFORM, "InputWin32 created");
}

InputWin32::~InputWin32() {
    LOG_TRACE(LogCategory::PLATFORM, "InputWin32 destroyed");
}

void InputWin32::setEventCallback(EventCallback callback) {
    eventCallback_ = callback;
}

int InputWin32::keyCodeToVK(KeyCode key) const {
    // Letters
    if (key >= KeyCode::A && key <= KeyCode::Z) {
        int offset = static_cast<int>(key) - static_cast<int>(KeyCode::A);
        return 'A' + offset;
    }

    // Numbers
    if (key >= KeyCode::Num0 && key <= KeyCode::Num9) {
        int offset = static_cast<int>(key) - static_cast<int>(KeyCode::Num0);
        return '0' + offset;
    }

    // Special keys
    switch (key) {
        case KeyCode::Space: return VK_SPACE;
        case KeyCode::Enter: return VK_RETURN;
        case KeyCode::Backspace: return VK_BACK;
        case KeyCode::Delete: return VK_DELETE;
        case KeyCode::Tab: return VK_TAB;
        case KeyCode::Left: return VK_LEFT;
        case KeyCode::Right: return VK_RIGHT;
        case KeyCode::Up: return VK_UP;
        case KeyCode::Down: return VK_DOWN;
        case KeyCode::Home: return VK_HOME;
        case KeyCode::End: return VK_END;
        case KeyCode::PageUp: return VK_PRIOR;
        case KeyCode::PageDown: return VK_NEXT;
        case KeyCode::Escape: return VK_ESCAPE;
        case KeyCode::F1: return VK_F1;
        case KeyCode::F2: return VK_F2;
        case KeyCode::F3: return VK_F3;
        case KeyCode::F4: return VK_F4;
        case KeyCode::F5: return VK_F5;
        case KeyCode::F6: return VK_F6;
        case KeyCode::F7: return VK_F7;
        case KeyCode::F8: return VK_F8;
        case KeyCode::F9: return VK_F9;
        case KeyCode::F10: return VK_F10;
        case KeyCode::F11: return VK_F11;
        case KeyCode::F12: return VK_F12;
        case KeyCode::LeftControl: return VK_LCONTROL;
        case KeyCode::RightControl: return VK_RCONTROL;
        case KeyCode::LeftShift: return VK_LSHIFT;
        case KeyCode::RightShift: return VK_RSHIFT;
        case KeyCode::LeftAlt: return VK_LMENU;
        case KeyCode::RightAlt: return VK_RMENU;
        default: return 0;
    }
}

bool InputWin32::isKeyPressed(KeyCode key) const {
    int vk = keyCodeToVK(key);
    if (vk == 0) {
        return false;
    }
    return (GetAsyncKeyState(vk) & 0x8000) != 0;
}

void InputWin32::getMousePosition(int& x, int& y) const {
    POINT pt;
    if (GetCursorPos(&pt)) {
        x = pt.x;
        y = pt.y;
    } else {
        x = 0;
        y = 0;
    }
}

} // namespace phantom
