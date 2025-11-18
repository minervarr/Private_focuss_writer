#ifndef PHANTOM_WINDOW_X11_H
#define PHANTOM_WINDOW_X11_H

#include "../platform_interface.h"
#include <X11/Xlib.h>
#include <vulkan/vulkan.h>
#include <functional>

namespace phantom {

class WindowX11 : public IPlatformWindow {
public:
    WindowX11();
    ~WindowX11() override;

    // IPlatformWindow interface
    bool create(const WindowConfig& config) override;
    void destroy() override;
    bool shouldClose() const override;
    void pollEvents() override;
    void getFramebufferSize(int& width, int& height) const override;
    bool isMinimized() const override;
    void setInputCallback(InputCallback callback) override { inputCallback_ = callback; }
    VkSurfaceKHR createVulkanSurface(VkInstance instance) override;
    const char** getRequiredVulkanExtensions(uint32_t& count) const override;

private:
    Display* display_ = nullptr;
    Window window_ = 0;
    Atom wmDeleteMessage_ = 0;
    bool shouldClose_ = false;
    bool isMinimized_ = false;
    int width_ = 0;
    int height_ = 0;
    InputCallback inputCallback_;
};

} // namespace phantom

#endif // PHANTOM_WINDOW_X11_H
