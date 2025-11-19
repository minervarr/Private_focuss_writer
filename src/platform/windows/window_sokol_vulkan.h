#ifndef PHANTOM_WINDOW_SOKOL_VULKAN_H
#define PHANTOM_WINDOW_SOKOL_VULKAN_H

#include "../platform_interface.h"
#include <vulkan/vulkan.h>
#include <functional>
#include <string>

// Forward declare sokol types to avoid header inclusion here
struct sapp_desc;
struct sapp_event;

namespace phantom {

/**
 * Windows window implementation using Sokol with Vulkan backend
 *
 * This provides a cleaner, more portable implementation compared to raw Win32.
 * Sokol handles all platform-specific details and provides a unified API.
 *
 * Features:
 * - Automatic Vulkan backend selection
 * - Built-in input handling
 * - Cross-platform event system
 * - Easy fullscreen support
 * - High DPI support
 */
class WindowSokolVulkan : public IPlatformWindow {
public:
    WindowSokolVulkan();
    ~WindowSokolVulkan() override;

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

    // Sokol-specific
    void handleSokolEvent(const sapp_event* event);
    sapp_desc getSokolDesc();

    // Singleton access (sokol requires C callbacks)
    static WindowSokolVulkan* instance() { return instance_; }

private:
    // Convert sokol key codes to our KeyCode
    KeyCode sokolKeyToKeyCode(int sapp_keycode) const;

    // Convert sokol mouse button to our MouseButton
    MouseButton sokolMouseToMouseButton(int sapp_mouse_button) const;

    static WindowSokolVulkan* instance_;

    WindowConfig config_;
    InputCallback inputCallback_;
    bool shouldClose_ = false;
    bool isMinimized_ = false;
    bool isFullscreen_ = false;
    int width_ = 0;
    int height_ = 0;
    VkInstance vulkanInstance_ = VK_NULL_HANDLE;
    VkSurfaceKHR vulkanSurface_ = VK_NULL_HANDLE;
};

} // namespace phantom

#endif // PHANTOM_WINDOW_SOKOL_VULKAN_H
