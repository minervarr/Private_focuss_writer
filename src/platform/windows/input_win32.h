#ifndef PHANTOM_INPUT_WIN32_H
#define PHANTOM_INPUT_WIN32_H

#include "../platform_interface.h"
#include <windows.h>

namespace phantom {

class InputWin32 : public IPlatformInput {
public:
    InputWin32();
    ~InputWin32() override;

    // IPlatformInput interface
    void setEventCallback(EventCallback callback) override;
    bool isKeyPressed(KeyCode key) const override;
    void getMousePosition(int& x, int& y) const override;

private:
    // Convert KeyCode to Windows virtual key
    int keyCodeToVK(KeyCode key) const;

    EventCallback eventCallback_;
};

} // namespace phantom

#endif // PHANTOM_INPUT_WIN32_H
