#ifndef PHANTOM_WINDOW_ANDROID_H
#define PHANTOM_WINDOW_ANDROID_H

#include "../platform_interface.h"
#include <android/native_window.h>
#include <android_native_app_glue.h>
#include <vulkan/vulkan.h>
#include <functional>

namespace phantom {

class WindowAndroid : public IPlatformWindow {
public:
    WindowAndroid();
    ~WindowAndroid() override;

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

    // Android-specific methods
    void setAndroidApp(android_app* app);
    void handleAppCommand(int32_t cmd);
    int32_t handleInputEvent(AInputEvent* event);

private:
    android_app* app_ = nullptr;
    ANativeWindow* nativeWindow_ = nullptr;
    bool shouldClose_ = false;
    bool isMinimized_ = false;
    bool isInitialized_ = false;
    int width_ = 0;
    int height_ = 0;
    InputCallback inputCallback_;
};

} // namespace phantom

#endif // PHANTOM_WINDOW_ANDROID_H
