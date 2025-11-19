#ifndef PLATFORM_INTERFACE_H
#define PLATFORM_INTERFACE_H

#include <cstdint>
#include <string>
#include <functional>
#include <vulkan/vulkan.h>

namespace phantom {

// Forward declarations
struct WindowConfig;
struct InputEvent;

// ============================================================================
// WINDOW INTERFACE
// ============================================================================

struct WindowConfig {
    std::string title = "Phantom Writer";
    int width = 1280;
    int height = 720;
    bool resizable = true;
    bool decorated = true;
    bool fullscreen = false;          // Fullscreen mode
    bool exclusiveFullscreen = false; // X11 exclusive fullscreen (overrides compositor)
};

class IPlatformWindow {
public:
    virtual ~IPlatformWindow() = default;

    // Lifecycle
    virtual bool create(const WindowConfig& config) = 0;
    virtual void destroy() = 0;
    virtual bool shouldClose() const = 0;

    // Event loop
    virtual void pollEvents() = 0;

    // Properties
    virtual void getFramebufferSize(int& width, int& height) const = 0;
    virtual bool isMinimized() const = 0;
    virtual bool isFullscreen() const = 0;

    // Fullscreen control
    virtual void setFullscreen(bool fullscreen, bool exclusive = false) = 0;
    virtual void toggleFullscreen() = 0;

    // Input callback
    using InputCallback = std::function<void(const InputEvent&)>;
    virtual void setInputCallback(InputCallback callback) = 0;

    // Vulkan integration
    virtual VkSurfaceKHR createVulkanSurface(VkInstance instance) = 0;
    virtual const char** getRequiredVulkanExtensions(uint32_t& count) const = 0;
};

// ============================================================================
// INPUT INTERFACE
// ============================================================================

enum class KeyCode {
    Unknown = 0,
    A, B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    Num0, Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
    Space, Enter, Backspace, Delete, Tab,
    Left, Right, Up, Down,
    Home, End, PageUp, PageDown,
    Escape, F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    LeftControl, RightControl,
    LeftShift, RightShift,
    LeftAlt, RightAlt,
};

enum class MouseButton {
    Left,
    Right,
    Middle,
};

struct InputEvent {
    enum class Type {
        KeyDown,
        KeyUp,
        KeyRepeat,
        MouseMove,
        MousePress,
        MouseRelease,
        MouseScroll,
        TouchDown,      // Android
        TouchUp,        // Android
        TouchMove,      // Android
        Character,      // Para text input
    } type;

    union {
        struct {
            KeyCode key;
            bool shift, ctrl, alt;
        } keyboard;

        struct {
            int x, y;
            int deltaX, deltaY;
        } mouse;

        struct {
            MouseButton button;
            int x, y;
        } mouseButton;

        struct {
            float deltaX, deltaY;
        } scroll;

        struct {
            int id;  // Touch pointer ID
            float x, y;
        } touch;

        struct {
            uint32_t codepoint;  // Unicode
        } character;
    } data;
};

class IPlatformInput {
public:
    virtual ~IPlatformInput() = default;

    // Callback registration
    using EventCallback = std::function<void(const InputEvent&)>;
    virtual void setEventCallback(EventCallback callback) = 0;

    // Polling (alternative to callbacks)
    virtual bool isKeyPressed(KeyCode key) const = 0;
    virtual void getMousePosition(int& x, int& y) const = 0;
};

// ============================================================================
// FILE SYSTEM INTERFACE
// ============================================================================

class IPlatformFileSystem {
public:
    virtual ~IPlatformFileSystem() = default;

    virtual std::string getConfigDirectory() const = 0;
    virtual std::string getDocumentsDirectory() const = 0;
    virtual std::string getTempDirectory() const = 0;

    virtual bool fileExists(const std::string& path) const = 0;
    virtual bool createDirectory(const std::string& path) const = 0;

    // Platform-specific separators
    virtual char getPathSeparator() const = 0;
};

// ============================================================================
// PLATFORM FACTORY
// ============================================================================

struct PlatformContext {
    IPlatformWindow* window = nullptr;
    IPlatformInput* input = nullptr;
    IPlatformFileSystem* fileSystem = nullptr;

    void cleanup() {
        delete window;
        delete input;
        delete fileSystem;
    }
};

// Factory function - implementada en cada plataforma
PlatformContext createPlatformContext();

} // namespace phantom

#endif // PLATFORM_INTERFACE_H
