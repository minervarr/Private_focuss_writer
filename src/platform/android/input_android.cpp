#include "input_android.h"
#include "utils/logger.h"

namespace phantom {

InputAndroid::InputAndroid() {
    LOG_TRACE(LogCategory::INPUT, "InputAndroid constructor");
}

InputAndroid::~InputAndroid() {
    LOG_TRACE(LogCategory::INPUT, "InputAndroid destructor");
}

void InputAndroid::setEventCallback(EventCallback callback) {
    eventCallback_ = callback;
}

bool InputAndroid::isKeyPressed(KeyCode key) const {
    // TODO: Implement key state tracking for Android
    (void)key;
    return false;
}

void InputAndroid::getMousePosition(int& x, int& y) const {
    // Android doesn't have a mouse, this is for touch
    // TODO: Return last touch position
    x = 0;
    y = 0;
}

} // namespace phantom
