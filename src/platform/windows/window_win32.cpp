#include "window_win32.h"
#include "utils/logger.h"
#include <vulkan/vulkan_win32.h>
#include <windowsx.h>

namespace phantom {

// Helper function to convert Win32 virtual key to our KeyCode
static KeyCode win32VKToKeyCode(WPARAM wParam) {
    // Letters
    if (wParam >= 'A' && wParam <= 'Z') {
        return static_cast<KeyCode>(static_cast<int>(KeyCode::A) + (wParam - 'A'));
    }

    // Numbers
    if (wParam >= '0' && wParam <= '9') {
        return static_cast<KeyCode>(static_cast<int>(KeyCode::Num0) + (wParam - '0'));
    }

    // Special keys
    switch (wParam) {
        case VK_SPACE: return KeyCode::Space;
        case VK_RETURN: return KeyCode::Enter;
        case VK_BACK: return KeyCode::Backspace;
        case VK_DELETE: return KeyCode::Delete;
        case VK_TAB: return KeyCode::Tab;
        case VK_LEFT: return KeyCode::Left;
        case VK_RIGHT: return KeyCode::Right;
        case VK_UP: return KeyCode::Up;
        case VK_DOWN: return KeyCode::Down;
        case VK_HOME: return KeyCode::Home;
        case VK_END: return KeyCode::End;
        case VK_PRIOR: return KeyCode::PageUp;
        case VK_NEXT: return KeyCode::PageDown;
        case VK_ESCAPE: return KeyCode::Escape;
        case VK_F1: return KeyCode::F1;
        case VK_F2: return KeyCode::F2;
        case VK_F3: return KeyCode::F3;
        case VK_F4: return KeyCode::F4;
        case VK_F5: return KeyCode::F5;
        case VK_F6: return KeyCode::F6;
        case VK_F7: return KeyCode::F7;
        case VK_F8: return KeyCode::F8;
        case VK_F9: return KeyCode::F9;
        case VK_F10: return KeyCode::F10;
        case VK_F11: return KeyCode::F11;
        case VK_F12: return KeyCode::F12;
        case VK_LCONTROL: return KeyCode::LeftControl;
        case VK_RCONTROL: return KeyCode::RightControl;
        case VK_LSHIFT: return KeyCode::LeftShift;
        case VK_RSHIFT: return KeyCode::RightShift;
        case VK_LMENU: return KeyCode::LeftAlt;
        case VK_RMENU: return KeyCode::RightAlt;
        default: return KeyCode::Unknown;
    }
}

WindowWin32::WindowWin32() {
    LOG_TRACE(LogCategory::PLATFORM, "WindowWin32 constructor");
    hInstance_ = GetModuleHandle(nullptr);
}

WindowWin32::~WindowWin32() {
    LOG_TRACE(LogCategory::PLATFORM, "WindowWin32 destructor");
    destroy();
}

LRESULT CALLBACK WindowWin32::windowProcStatic(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    WindowWin32* window = nullptr;

    if (msg == WM_NCCREATE) {
        // Store pointer to WindowWin32 instance
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        window = reinterpret_cast<WindowWin32*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
    } else {
        // Retrieve pointer to WindowWin32 instance
        window = reinterpret_cast<WindowWin32*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    }

    if (window) {
        return window->windowProc(hwnd, msg, wParam, lParam);
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT WindowWin32::windowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CLOSE:
            shouldClose_ = true;
            return 0;

        case WM_SIZE: {
            width_ = LOWORD(lParam);
            height_ = HIWORD(lParam);

            if (wParam == SIZE_MINIMIZED) {
                isMinimized_ = true;
            } else if (wParam == SIZE_RESTORED || wParam == SIZE_MAXIMIZED) {
                isMinimized_ = false;
            }
            return 0;
        }

        case WM_KEYDOWN:
        case WM_SYSKEYDOWN: {
            if (inputCallback_) {
                InputEvent event;
                event.type = InputEvent::Type::KeyDown;
                event.data.keyboard.key = win32VKToKeyCode(wParam);
                event.data.keyboard.shift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
                event.data.keyboard.ctrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
                event.data.keyboard.alt = (GetKeyState(VK_MENU) & 0x8000) != 0;
                inputCallback_(event);
            }
            return 0;
        }

        case WM_KEYUP:
        case WM_SYSKEYUP: {
            if (inputCallback_) {
                InputEvent event;
                event.type = InputEvent::Type::KeyUp;
                event.data.keyboard.key = win32VKToKeyCode(wParam);
                event.data.keyboard.shift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
                event.data.keyboard.ctrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
                event.data.keyboard.alt = (GetKeyState(VK_MENU) & 0x8000) != 0;
                inputCallback_(event);
            }
            return 0;
        }

        case WM_CHAR: {
            if (inputCallback_) {
                // Filter out control characters
                if (wParam >= 32 && wParam != 127) {
                    InputEvent event;
                    event.type = InputEvent::Type::Character;
                    event.data.character.codepoint = static_cast<uint32_t>(wParam);
                    inputCallback_(event);
                }
            }
            return 0;
        }

        case WM_MOUSEMOVE: {
            if (inputCallback_) {
                InputEvent event;
                event.type = InputEvent::Type::MouseMove;
                event.data.mouse.x = GET_X_LPARAM(lParam);
                event.data.mouse.y = GET_Y_LPARAM(lParam);
                event.data.mouse.deltaX = 0;
                event.data.mouse.deltaY = 0;
                inputCallback_(event);
            }
            return 0;
        }

        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN: {
            if (inputCallback_) {
                InputEvent event;
                event.type = InputEvent::Type::MousePress;
                event.data.mouseButton.x = GET_X_LPARAM(lParam);
                event.data.mouseButton.y = GET_Y_LPARAM(lParam);

                if (msg == WM_LBUTTONDOWN) event.data.mouseButton.button = MouseButton::Left;
                else if (msg == WM_RBUTTONDOWN) event.data.mouseButton.button = MouseButton::Right;
                else event.data.mouseButton.button = MouseButton::Middle;

                inputCallback_(event);
            }
            return 0;
        }

        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP: {
            if (inputCallback_) {
                InputEvent event;
                event.type = InputEvent::Type::MouseRelease;
                event.data.mouseButton.x = GET_X_LPARAM(lParam);
                event.data.mouseButton.y = GET_Y_LPARAM(lParam);

                if (msg == WM_LBUTTONUP) event.data.mouseButton.button = MouseButton::Left;
                else if (msg == WM_RBUTTONUP) event.data.mouseButton.button = MouseButton::Right;
                else event.data.mouseButton.button = MouseButton::Middle;

                inputCallback_(event);
            }
            return 0;
        }

        case WM_MOUSEWHEEL: {
            if (inputCallback_) {
                InputEvent event;
                event.type = InputEvent::Type::MouseScroll;
                event.data.scroll.deltaX = 0.0f;
                event.data.scroll.deltaY = GET_WHEEL_DELTA_WPARAM(wParam) / static_cast<float>(WHEEL_DELTA);
                inputCallback_(event);
            }
            return 0;
        }
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

bool WindowWin32::create(const WindowConfig& config) {
    LOG_INFO(LogCategory::PLATFORM, "Creating Win32 window: %s (%dx%d)",
        config.title.c_str(), config.width, config.height);

    // Register window class
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = windowProcStatic;
    wc.hInstance = hInstance_;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wc.lpszClassName = "PhantomWriterWindowClass";

    if (!RegisterClassEx(&wc)) {
        LOG_ERROR(LogCategory::PLATFORM, "Failed to register window class");
        return false;
    }

    LOG_DEBUG(LogCategory::PLATFORM, "Window class registered successfully");

    // Calculate window size with decorations
    DWORD style = config.decorated ?
        (WS_OVERLAPPEDWINDOW) :
        (WS_POPUP | WS_VISIBLE);

    if (!config.resizable && config.decorated) {
        style &= ~(WS_THICKFRAME | WS_MAXIMIZEBOX);
    }

    RECT rect = { 0, 0, config.width, config.height };
    AdjustWindowRect(&rect, style, FALSE);

    width_ = config.width;
    height_ = config.height;

    // Create window
    hwnd_ = CreateWindowEx(
        0,
        "PhantomWriterWindowClass",
        config.title.c_str(),
        style,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left,
        rect.bottom - rect.top,
        nullptr,
        nullptr,
        hInstance_,
        this  // Pass 'this' pointer to WM_NCCREATE
    );

    if (!hwnd_) {
        LOG_ERROR(LogCategory::PLATFORM, "Failed to create Win32 window");
        return false;
    }

    LOG_DEBUG(LogCategory::PLATFORM, "Win32 window created successfully");

    // Show window
    ShowWindow(hwnd_, SW_SHOW);
    UpdateWindow(hwnd_);

    LOG_INFO(LogCategory::PLATFORM, "Win32 window created and shown");
    return true;
}

void WindowWin32::destroy() {
    if (hwnd_) {
        DestroyWindow(hwnd_);
        hwnd_ = nullptr;
        LOG_DEBUG(LogCategory::PLATFORM, "Win32 window destroyed");
    }

    UnregisterClass("PhantomWriterWindowClass", hInstance_);
}

bool WindowWin32::shouldClose() const {
    return shouldClose_;
}

void WindowWin32::pollEvents() {
    MSG msg;
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void WindowWin32::getFramebufferSize(int& width, int& height) const {
    width = width_;
    height = height_;
}

bool WindowWin32::isMinimized() const {
    return isMinimized_;
}

VkSurfaceKHR WindowWin32::createVulkanSurface(VkInstance instance) {
    VkWin32SurfaceCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hinstance = hInstance_;
    createInfo.hwnd = hwnd_;

    VkSurfaceKHR surface;
    VkResult result = vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &surface);

    if (result != VK_SUCCESS) {
        LOG_ERROR(LogCategory::PLATFORM, "Failed to create Vulkan Win32 surface");
        return VK_NULL_HANDLE;
    }

    LOG_INFO(LogCategory::PLATFORM, "Vulkan Win32 surface created successfully");
    return surface;
}

const char** WindowWin32::getRequiredVulkanExtensions(uint32_t& count) const {
    static const char* extensions[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME
    };
    count = 2;
    return extensions;
}

} // namespace phantom
