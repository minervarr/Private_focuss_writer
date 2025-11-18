#include "window_android.h"
#include "utils/logger.h"
#include <vulkan/vulkan_android.h>

namespace phantom {

WindowAndroid::WindowAndroid() {
    LOG_TRACE(LogCategory::PLATFORM, "WindowAndroid constructor");
}

WindowAndroid::~WindowAndroid() {
    LOG_TRACE(LogCategory::PLATFORM, "WindowAndroid destructor");
    destroy();
}

void WindowAndroid::setAndroidApp(android_app* app) {
    app_ = app;
    LOG_DEBUG(LogCategory::PLATFORM, "Android app set");
}

bool WindowAndroid::create(const WindowConfig& config) {
    LOG_INFO(LogCategory::PLATFORM, "Creating Android window: %s", config.title.c_str());

    if (!app_ || !app_->window) {
        LOG_ERROR(LogCategory::PLATFORM, "Android app or window not initialized");
        return false;
    }

    nativeWindow_ = app_->window;

    // Get window dimensions
    width_ = ANativeWindow_getWidth(nativeWindow_);
    height_ = ANativeWindow_getHeight(nativeWindow_);

    LOG_INFO(LogCategory::PLATFORM, "Android window created: %dx%d", width_, height_);

    isInitialized_ = true;
    return true;
}

void WindowAndroid::destroy() {
    if (nativeWindow_) {
        LOG_INFO(LogCategory::PLATFORM, "Destroying Android window");
        nativeWindow_ = nullptr;
    }
    isInitialized_ = false;
}

bool WindowAndroid::shouldClose() const {
    return shouldClose_;
}

void WindowAndroid::pollEvents() {
    // Events are handled via callbacks in Android
    // The main loop calls android_app_glue's poll source
}

void WindowAndroid::getFramebufferSize(int& width, int& height) const {
    width = width_;
    height = height_;
}

bool WindowAndroid::isMinimized() const {
    return isMinimized_;
}

bool WindowAndroid::isFullscreen() const {
    // Android apps are always fullscreen by default
    return true;
}

void WindowAndroid::setFullscreen(bool fullscreen, bool exclusive) {
    // Android apps are always fullscreen
    (void)fullscreen;
    (void)exclusive;
    LOG_DEBUG(LogCategory::PLATFORM, "Android windows are always fullscreen");
}

void WindowAndroid::toggleFullscreen() {
    // No-op on Android
}

void WindowAndroid::handleAppCommand(int32_t cmd) {
    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            if (app_->window != nullptr) {
                nativeWindow_ = app_->window;
                width_ = ANativeWindow_getWidth(nativeWindow_);
                height_ = ANativeWindow_getHeight(nativeWindow_);
                isMinimized_ = false;
                LOG_INFO(LogCategory::PLATFORM, "Window initialized: %dx%d", width_, height_);
            }
            break;

        case APP_CMD_TERM_WINDOW:
            nativeWindow_ = nullptr;
            LOG_INFO(LogCategory::PLATFORM, "Window terminated");
            break;

        case APP_CMD_WINDOW_RESIZED:
            if (nativeWindow_) {
                width_ = ANativeWindow_getWidth(nativeWindow_);
                height_ = ANativeWindow_getHeight(nativeWindow_);
                LOG_DEBUG(LogCategory::PLATFORM, "Window resized: %dx%d", width_, height_);
            }
            break;

        case APP_CMD_GAINED_FOCUS:
            isMinimized_ = false;
            LOG_DEBUG(LogCategory::PLATFORM, "Gained focus");
            break;

        case APP_CMD_LOST_FOCUS:
            isMinimized_ = true;
            LOG_DEBUG(LogCategory::PLATFORM, "Lost focus");
            break;

        case APP_CMD_PAUSE:
            isMinimized_ = true;
            LOG_INFO(LogCategory::PLATFORM, "App paused");
            break;

        case APP_CMD_RESUME:
            isMinimized_ = false;
            LOG_INFO(LogCategory::PLATFORM, "App resumed");
            break;

        case APP_CMD_STOP:
            LOG_INFO(LogCategory::PLATFORM, "App stopped");
            break;

        case APP_CMD_DESTROY:
            shouldClose_ = true;
            LOG_INFO(LogCategory::PLATFORM, "App destroyed");
            break;

        default:
            break;
    }
}

int32_t WindowAndroid::handleInputEvent(AInputEvent* event) {
    if (!inputCallback_) {
        return 0;
    }

    int32_t eventType = AInputEvent_getType(event);

    if (eventType == AINPUT_EVENT_TYPE_MOTION) {
        int32_t action = AMotionEvent_getAction(event);
        int32_t actionMasked = action & AMOTION_EVENT_ACTION_MASK;

        float x = AMotionEvent_getX(event, 0);
        float y = AMotionEvent_getY(event, 0);

        InputEvent inputEvent;

        switch (actionMasked) {
            case AMOTION_EVENT_ACTION_DOWN:
                inputEvent.type = InputEvent::Type::TouchDown;
                inputEvent.data.touch.id = AMotionEvent_getPointerId(event, 0);
                inputEvent.data.touch.x = x;
                inputEvent.data.touch.y = y;
                inputCallback_(inputEvent);
                return 1;

            case AMOTION_EVENT_ACTION_UP:
                inputEvent.type = InputEvent::Type::TouchUp;
                inputEvent.data.touch.id = AMotionEvent_getPointerId(event, 0);
                inputEvent.data.touch.x = x;
                inputEvent.data.touch.y = y;
                inputCallback_(inputEvent);
                return 1;

            case AMOTION_EVENT_ACTION_MOVE:
                inputEvent.type = InputEvent::Type::TouchMove;
                inputEvent.data.touch.id = AMotionEvent_getPointerId(event, 0);
                inputEvent.data.touch.x = x;
                inputEvent.data.touch.y = y;
                inputCallback_(inputEvent);
                return 1;

            default:
                break;
        }
    }
    else if (eventType == AINPUT_EVENT_TYPE_KEY) {
        // Handle key events
        int32_t keyCode = AKeyEvent_getKeyCode(event);
        int32_t action = AKeyEvent_getAction(event);

        // TODO: Map Android key codes to our KeyCode enum
        (void)keyCode;
        (void)action;
    }

    return 0;
}

VkSurfaceKHR WindowAndroid::createVulkanSurface(VkInstance instance) {
    LOG_INFO(LogCategory::VULKAN, "Creating Vulkan surface for Android");

    if (!nativeWindow_) {
        LOG_ERROR(LogCategory::VULKAN, "Native window not available");
        return VK_NULL_HANDLE;
    }

    VkAndroidSurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    createInfo.window = nativeWindow_;

    VkSurfaceKHR surface;
    VkResult result = vkCreateAndroidSurfaceKHR(instance, &createInfo, nullptr, &surface);

    if (result != VK_SUCCESS) {
        LOG_ERROR(LogCategory::VULKAN, "Failed to create Vulkan Android surface: %d", result);
        return VK_NULL_HANDLE;
    }

    LOG_INFO(LogCategory::VULKAN, "Vulkan Android surface created successfully");
    return surface;
}

const char** WindowAndroid::getRequiredVulkanExtensions(uint32_t& count) const {
    static const char* extensions[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_ANDROID_SURFACE_EXTENSION_NAME
    };

    count = 2;
    return extensions;
}

} // namespace phantom
