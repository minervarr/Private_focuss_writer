#ifndef PHANTOM_WINDOW_WIN32_H
#define PHANTOM_WINDOW_WIN32_H

#include "../platform_interface.h"
#include <windows.h>
#include <vulkan/vulkan.h>
#include <functional>

namespace phantom {

class WindowWin32 : public IPlatformWindow {
public:
    WindowWin32();
    ~WindowWin32() override;

    // IPlatformWindow interface
    bool create(const WindowConfig& config) override;
    void destroy() override;
    bool shouldClose() const override;
    void pollEvents() override;
    void getFramebufferSize(int& width, int& height) const override;
    bool isMinimized() const override;
    bool isFullscreen() const override;
    void setFullscreen(bool fullscreen, bool exclusive = false) override;
    void toggleFullscreen() override;
    void setInputCallback(InputCallback callback) override { inputCallback_ = callback; }
    VkSurfaceKHR createVulkanSurface(VkInstance instance) override;
    const char** getRequiredVulkanExtensions(uint32_t& count) const override;

    // Get HWND for Vulkan surface creation
    HWND getHWND() const { return hwnd_; }
    HINSTANCE getHInstance() const { return hInstance_; }

private:
    // Window procedure (static)
    static LRESULT CALLBACK windowProcStatic(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    // Instance window procedure
    LRESULT windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    HINSTANCE hInstance_ = nullptr;
    HWND hwnd_ = nullptr;
    bool shouldClose_ = false;
    bool isMinimized_ = false;
    int width_ = 0;
    int height_ = 0;
    InputCallback inputCallback_;
};

} // namespace phantom

#endif // PHANTOM_WINDOW_WIN32_H
