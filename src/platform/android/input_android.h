#ifndef PHANTOM_INPUT_ANDROID_H
#define PHANTOM_INPUT_ANDROID_H

#include "../platform_interface.h"

namespace phantom {

class InputAndroid : public IPlatformInput {
public:
    InputAndroid();
    ~InputAndroid() override;

    // IPlatformInput interface
    void setEventCallback(EventCallback callback) override;
    bool isKeyPressed(KeyCode key) const override;
    void getMousePosition(int& x, int& y) const override;

private:
    EventCallback eventCallback_;
};

} // namespace phantom

#endif // PHANTOM_INPUT_ANDROID_H
