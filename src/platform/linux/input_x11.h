#ifndef PHANTOM_INPUT_X11_H
#define PHANTOM_INPUT_X11_H

#include "../platform_interface.h"

namespace phantom {

class InputX11 : public IPlatformInput {
public:
    InputX11();
    ~InputX11() override;

    // IPlatformInput interface
    void setEventCallback(EventCallback callback) override;
    bool isKeyPressed(KeyCode key) const override;
    void getMousePosition(int& x, int& y) const override;

private:
    EventCallback eventCallback_;
    int mouseX_ = 0;
    int mouseY_ = 0;
};

} // namespace phantom

#endif // PHANTOM_INPUT_X11_H
