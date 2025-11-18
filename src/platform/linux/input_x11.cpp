#include "input_x11.h"
#include "utils/logger.h"

namespace phantom {

InputX11::InputX11() {
    LOG_TRACE(LogCategory::INPUT, "InputX11 constructor");
}

InputX11::~InputX11() {
    LOG_TRACE(LogCategory::INPUT, "InputX11 destructor");
}

void InputX11::setEventCallback(EventCallback callback) {
    eventCallback_ = callback;
}

bool InputX11::isKeyPressed(KeyCode key) const {
    // TODO: Implementar en fase posterior
    return false;
}

void InputX11::getMousePosition(int& x, int& y) const {
    x = mouseX_;
    y = mouseY_;
}

} // namespace phantom
