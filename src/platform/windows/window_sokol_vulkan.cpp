#include "window_sokol_vulkan.h"
#include "utils/logger.h"

// Define sokol implementation in this file
#define SOKOL_IMPL
#define SOKOL_WIN32
#define SOKOL_D3D11  // Sokol requires a backend, we'll use D3D11 as fallback
#define SOKOL_GLCORE33  // And GL as another fallback
#define SOKOL_NO_DEPRECATED

// Include sokol headers
#include "../../third_party/sokol/sokol_app.h"
#include "../../third_party/sokol/sokol_gfx.h"
#include "../../third_party/sokol/sokol_glue.h"

// Vulkan support
#include <vulkan/vulkan.h>
#ifdef _WIN32
#include <vulkan/vulkan_win32.h>
#endif

namespace phantom {

// Static instance pointer
WindowSokolVulkan* WindowSokolVulkan::instance_ = nullptr;

// Sokol callbacks (must be C-style)
extern "C" {

static void sokol_init_cb() {
    if (WindowSokolVulkan::instance()) {
        LOG_INFO(LogCategory::PLATFORM, "Sokol initialized");
    }
}

static void sokol_frame_cb() {
    // Frame callback - not used in our architecture
    // We handle rendering externally
}

static void sokol_cleanup_cb() {
    if (WindowSokolVulkan::instance()) {
        LOG_INFO(LogCategory::PLATFORM, "Sokol cleanup");
    }
}

static void sokol_event_cb(const sapp_event* event) {
    if (WindowSokolVulkan::instance()) {
        WindowSokolVulkan::instance()->handleSokolEvent(event);
    }
}

} // extern "C"

WindowSokolVulkan::WindowSokolVulkan() {
    LOG_TRACE(LogCategory::PLATFORM, "WindowSokolVulkan constructor");
    instance_ = this;
}

WindowSokolVulkan::~WindowSokolVulkan() {
    LOG_TRACE(LogCategory::PLATFORM, "WindowSokolVulkan destructor");
    destroy();
    if (instance_ == this) {
        instance_ = nullptr;
    }
}

KeyCode WindowSokolVulkan::sokolKeyToKeyCode(int sapp_keycode) const {
    switch (sapp_keycode) {
        case SAPP_KEYCODE_SPACE: return KeyCode::Space;
        case SAPP_KEYCODE_APOSTROPHE: return KeyCode::Unknown; // '
        case SAPP_KEYCODE_COMMA: return KeyCode::Unknown; // ,
        case SAPP_KEYCODE_MINUS: return KeyCode::Unknown; // -
        case SAPP_KEYCODE_PERIOD: return KeyCode::Unknown; // .
        case SAPP_KEYCODE_SLASH: return KeyCode::Unknown; // /
        case SAPP_KEYCODE_0: return KeyCode::Num0;
        case SAPP_KEYCODE_1: return KeyCode::Num1;
        case SAPP_KEYCODE_2: return KeyCode::Num2;
        case SAPP_KEYCODE_3: return KeyCode::Num3;
        case SAPP_KEYCODE_4: return KeyCode::Num4;
        case SAPP_KEYCODE_5: return KeyCode::Num5;
        case SAPP_KEYCODE_6: return KeyCode::Num6;
        case SAPP_KEYCODE_7: return KeyCode::Num7;
        case SAPP_KEYCODE_8: return KeyCode::Num8;
        case SAPP_KEYCODE_9: return KeyCode::Num9;
        case SAPP_KEYCODE_SEMICOLON: return KeyCode::Unknown; // ;
        case SAPP_KEYCODE_EQUAL: return KeyCode::Unknown; // =
        case SAPP_KEYCODE_A: return KeyCode::A;
        case SAPP_KEYCODE_B: return KeyCode::B;
        case SAPP_KEYCODE_C: return KeyCode::C;
        case SAPP_KEYCODE_D: return KeyCode::D;
        case SAPP_KEYCODE_E: return KeyCode::E;
        case SAPP_KEYCODE_F: return KeyCode::F;
        case SAPP_KEYCODE_G: return KeyCode::G;
        case SAPP_KEYCODE_H: return KeyCode::H;
        case SAPP_KEYCODE_I: return KeyCode::I;
        case SAPP_KEYCODE_J: return KeyCode::J;
        case SAPP_KEYCODE_K: return KeyCode::K;
        case SAPP_KEYCODE_L: return KeyCode::L;
        case SAPP_KEYCODE_M: return KeyCode::M;
        case SAPP_KEYCODE_N: return KeyCode::N;
        case SAPP_KEYCODE_O: return KeyCode::O;
        case SAPP_KEYCODE_P: return KeyCode::P;
        case SAPP_KEYCODE_Q: return KeyCode::Q;
        case SAPP_KEYCODE_R: return KeyCode::R;
        case SAPP_KEYCODE_S: return KeyCode::S;
        case SAPP_KEYCODE_T: return KeyCode::T;
        case SAPP_KEYCODE_U: return KeyCode::U;
        case SAPP_KEYCODE_V: return KeyCode::V;
        case SAPP_KEYCODE_W: return KeyCode::W;
        case SAPP_KEYCODE_X: return KeyCode::X;
        case SAPP_KEYCODE_Y: return KeyCode::Y;
        case SAPP_KEYCODE_Z: return KeyCode::Z;
        case SAPP_KEYCODE_LEFT_BRACKET: return KeyCode::Unknown; // [
        case SAPP_KEYCODE_BACKSLASH: return KeyCode::Unknown; // \
        case SAPP_KEYCODE_RIGHT_BRACKET: return KeyCode::Unknown; // ]
        case SAPP_KEYCODE_GRAVE_ACCENT: return KeyCode::Unknown; // `
        case SAPP_KEYCODE_ESCAPE: return KeyCode::Escape;
        case SAPP_KEYCODE_ENTER: return KeyCode::Enter;
        case SAPP_KEYCODE_TAB: return KeyCode::Tab;
        case SAPP_KEYCODE_BACKSPACE: return KeyCode::Backspace;
        case SAPP_KEYCODE_INSERT: return KeyCode::Insert;
        case SAPP_KEYCODE_DELETE: return KeyCode::Delete;
        case SAPP_KEYCODE_RIGHT: return KeyCode::Right;
        case SAPP_KEYCODE_LEFT: return KeyCode::Left;
        case SAPP_KEYCODE_DOWN: return KeyCode::Down;
        case SAPP_KEYCODE_UP: return KeyCode::Up;
        case SAPP_KEYCODE_PAGE_UP: return KeyCode::PageUp;
        case SAPP_KEYCODE_PAGE_DOWN: return KeyCode::PageDown;
        case SAPP_KEYCODE_HOME: return KeyCode::Home;
        case SAPP_KEYCODE_END: return KeyCode::End;
        case SAPP_KEYCODE_F1: return KeyCode::F1;
        case SAPP_KEYCODE_F2: return KeyCode::F2;
        case SAPP_KEYCODE_F3: return KeyCode::F3;
        case SAPP_KEYCODE_F4: return KeyCode::F4;
        case SAPP_KEYCODE_F5: return KeyCode::F5;
        case SAPP_KEYCODE_F6: return KeyCode::F6;
        case SAPP_KEYCODE_F7: return KeyCode::F7;
        case SAPP_KEYCODE_F8: return KeyCode::F8;
        case SAPP_KEYCODE_F9: return KeyCode::F9;
        case SAPP_KEYCODE_F10: return KeyCode::F10;
        case SAPP_KEYCODE_F11: return KeyCode::F11;
        case SAPP_KEYCODE_F12: return KeyCode::F12;
        case SAPP_KEYCODE_LEFT_SHIFT: return KeyCode::LeftShift;
        case SAPP_KEYCODE_LEFT_CONTROL: return KeyCode::LeftControl;
        case SAPP_KEYCODE_LEFT_ALT: return KeyCode::LeftAlt;
        case SAPP_KEYCODE_RIGHT_SHIFT: return KeyCode::RightShift;
        case SAPP_KEYCODE_RIGHT_CONTROL: return KeyCode::RightControl;
        case SAPP_KEYCODE_RIGHT_ALT: return KeyCode::RightAlt;
        default: return KeyCode::Unknown;
    }
}

MouseButton WindowSokolVulkan::sokolMouseToMouseButton(int sapp_mouse_button) const {
    switch (sapp_mouse_button) {
        case SAPP_MOUSEBUTTON_LEFT: return MouseButton::Left;
        case SAPP_MOUSEBUTTON_RIGHT: return MouseButton::Right;
        case SAPP_MOUSEBUTTON_MIDDLE: return MouseButton::Middle;
        default: return MouseButton::Left;
    }
}

void WindowSokolVulkan::handleSokolEvent(const sapp_event* event) {
    if (!inputCallback_) {
        return;
    }

    switch (event->type) {
        case SAPP_EVENTTYPE_KEY_DOWN: {
            InputEvent inputEvent;
            inputEvent.type = InputEvent::Type::KeyDown;
            inputEvent.data.keyboard.key = sokolKeyToKeyCode(event->key_code);
            inputEvent.data.keyboard.shift = (event->modifiers & SAPP_MODIFIER_SHIFT) != 0;
            inputEvent.data.keyboard.ctrl = (event->modifiers & SAPP_MODIFIER_CTRL) != 0;
            inputEvent.data.keyboard.alt = (event->modifiers & SAPP_MODIFIER_ALT) != 0;
            inputCallback_(inputEvent);
            break;
        }

        case SAPP_EVENTTYPE_KEY_UP: {
            InputEvent inputEvent;
            inputEvent.type = InputEvent::Type::KeyUp;
            inputEvent.data.keyboard.key = sokolKeyToKeyCode(event->key_code);
            inputEvent.data.keyboard.shift = (event->modifiers & SAPP_MODIFIER_SHIFT) != 0;
            inputEvent.data.keyboard.ctrl = (event->modifiers & SAPP_MODIFIER_CTRL) != 0;
            inputEvent.data.keyboard.alt = (event->modifiers & SAPP_MODIFIER_ALT) != 0;
            inputCallback_(inputEvent);
            break;
        }

        case SAPP_EVENTTYPE_CHAR: {
            // Character input
            if (event->char_code >= 32 && event->char_code != 127) {
                InputEvent inputEvent;
                inputEvent.type = InputEvent::Type::Character;
                inputEvent.data.character.codepoint = event->char_code;
                inputCallback_(inputEvent);
            }
            break;
        }

        case SAPP_EVENTTYPE_MOUSE_DOWN: {
            InputEvent inputEvent;
            inputEvent.type = InputEvent::Type::MousePress;
            inputEvent.data.mouseButton.x = static_cast<int>(event->mouse_x);
            inputEvent.data.mouseButton.y = static_cast<int>(event->mouse_y);
            inputEvent.data.mouseButton.button = sokolMouseToMouseButton(event->mouse_button);
            inputCallback_(inputEvent);
            break;
        }

        case SAPP_EVENTTYPE_MOUSE_UP: {
            InputEvent inputEvent;
            inputEvent.type = InputEvent::Type::MouseRelease;
            inputEvent.data.mouseButton.x = static_cast<int>(event->mouse_x);
            inputEvent.data.mouseButton.y = static_cast<int>(event->mouse_y);
            inputEvent.data.mouseButton.button = sokolMouseToMouseButton(event->mouse_button);
            inputCallback_(inputEvent);
            break;
        }

        case SAPP_EVENTTYPE_MOUSE_MOVE: {
            InputEvent inputEvent;
            inputEvent.type = InputEvent::Type::MouseMove;
            inputEvent.data.mouse.x = static_cast<int>(event->mouse_x);
            inputEvent.data.mouse.y = static_cast<int>(event->mouse_y);
            inputEvent.data.mouse.deltaX = static_cast<int>(event->mouse_dx);
            inputEvent.data.mouse.deltaY = static_cast<int>(event->mouse_dy);
            inputCallback_(inputEvent);
            break;
        }

        case SAPP_EVENTTYPE_MOUSE_SCROLL: {
            InputEvent inputEvent;
            inputEvent.type = InputEvent::Type::MouseScroll;
            inputEvent.data.scroll.deltaX = event->scroll_x;
            inputEvent.data.scroll.deltaY = event->scroll_y;
            inputCallback_(inputEvent);
            break;
        }

        case SAPP_EVENTTYPE_RESIZED: {
            width_ = sapp_width();
            height_ = sapp_height();
            LOG_DEBUG(LogCategory::PLATFORM, "Window resized: %dx%d", width_, height_);
            break;
        }

        case SAPP_EVENTTYPE_ICONIFIED: {
            isMinimized_ = true;
            LOG_DEBUG(LogCategory::PLATFORM, "Window minimized");
            break;
        }

        case SAPP_EVENTTYPE_RESTORED: {
            isMinimized_ = false;
            LOG_DEBUG(LogCategory::PLATFORM, "Window restored");
            break;
        }

        case SAPP_EVENTTYPE_QUIT_REQUESTED: {
            shouldClose_ = true;
            LOG_INFO(LogCategory::PLATFORM, "Window close requested");
            break;
        }

        default:
            break;
    }
}

sapp_desc WindowSokolVulkan::getSokolDesc() {
    sapp_desc desc = {};
    desc.init_cb = sokol_init_cb;
    desc.frame_cb = sokol_frame_cb;
    desc.cleanup_cb = sokol_cleanup_cb;
    desc.event_cb = sokol_event_cb;
    desc.width = config_.width;
    desc.height = config_.height;
    desc.window_title = config_.title.c_str();
    desc.high_dpi = true;
    desc.fullscreen = false;
    desc.sample_count = 1;
    return desc;
}

bool WindowSokolVulkan::create(const WindowConfig& config) {
    LOG_INFO(LogCategory::PLATFORM, "Creating Sokol+Vulkan window: %s (%dx%d)",
        config.title.c_str(), config.width, config.height);

    config_ = config;
    width_ = config.width;
    height_ = config.height;

    // Note: With sokol_app, the window creation happens via sapp_run()
    // which needs to be called from main(). This is just configuration.
    LOG_INFO(LogCategory::PLATFORM, "Sokol window configured (call sapp_run in main)");
    return true;
}

void WindowSokolVulkan::destroy() {
    if (vulkanSurface_ != VK_NULL_HANDLE && vulkanInstance_ != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(vulkanInstance_, vulkanSurface_, nullptr);
        vulkanSurface_ = VK_NULL_HANDLE;
        LOG_DEBUG(LogCategory::PLATFORM, "Vulkan surface destroyed");
    }

    // Sokol cleanup happens automatically in sapp_cleanup
    LOG_DEBUG(LogCategory::PLATFORM, "Sokol window destroyed");
}

bool WindowSokolVulkan::shouldClose() const {
    return shouldClose_ || !sapp_isvalid();
}

void WindowSokolVulkan::pollEvents() {
    // Sokol handles events automatically in its main loop
    // This is called from our main loop to maintain compatibility
}

void WindowSokolVulkan::getFramebufferSize(int& width, int& height) const {
    width = sapp_width();
    height = sapp_height();
}

bool WindowSokolVulkan::isMinimized() const {
    return isMinimized_;
}

bool WindowSokolVulkan::isFullscreen() const {
    return sapp_is_fullscreen();
}

void WindowSokolVulkan::setFullscreen(bool fullscreen, bool exclusive) {
    (void)exclusive; // Sokol doesn't distinguish exclusive vs borderless
    sapp_toggle_fullscreen();
    isFullscreen_ = fullscreen;
    LOG_INFO(LogCategory::PLATFORM, "Fullscreen: %s", fullscreen ? "ON" : "OFF");
}

void WindowSokolVulkan::toggleFullscreen() {
    sapp_toggle_fullscreen();
    isFullscreen_ = !isFullscreen_;
    LOG_INFO(LogCategory::PLATFORM, "Fullscreen toggled: %s", isFullscreen_ ? "ON" : "OFF");
}

VkSurfaceKHR WindowSokolVulkan::createVulkanSurface(VkInstance instance) {
    vulkanInstance_ = instance;

    // Get native window handle from sokol
    #ifdef _WIN32
    HWND hwnd = (HWND)sapp_win32_get_hwnd();
    HINSTANCE hinstance = GetModuleHandle(nullptr);

    VkWin32SurfaceCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hinstance = hinstance;
    createInfo.hwnd = hwnd;

    VkResult result = vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &vulkanSurface_);

    if (result != VK_SUCCESS) {
        LOG_ERROR(LogCategory::PLATFORM, "Failed to create Vulkan Win32 surface from Sokol window");
        return VK_NULL_HANDLE;
    }

    LOG_INFO(LogCategory::PLATFORM, "Vulkan surface created from Sokol window");
    return vulkanSurface_;
    #else
    LOG_ERROR(LogCategory::PLATFORM, "Platform not supported for Vulkan surface creation");
    return VK_NULL_HANDLE;
    #endif
}

const char** WindowSokolVulkan::getRequiredVulkanExtensions(uint32_t& count) const {
    static const char* extensions[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        #ifdef _WIN32
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME
        #endif
    };
    count = sizeof(extensions) / sizeof(extensions[0]);
    return extensions;
}

} // namespace phantom
