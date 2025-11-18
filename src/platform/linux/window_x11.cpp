#include "window_x11.h"
#include "utils/logger.h"
#include <vulkan/vulkan_xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <cstring>

// X11 defines KeyPress/KeyRelease as macros which conflict with our enum
#ifdef KeyPress
#undef KeyPress
#endif
#ifdef KeyRelease
#undef KeyRelease
#endif

namespace phantom {

// Helper function to convert X11 KeySym to our KeyCode
static KeyCode x11KeySymToKeyCode(KeySym keysym) {
    // Letters
    if (keysym >= XK_a && keysym <= XK_z) {
        return static_cast<KeyCode>(static_cast<int>(KeyCode::A) + (keysym - XK_a));
    }
    if (keysym >= XK_A && keysym <= XK_Z) {
        return static_cast<KeyCode>(static_cast<int>(KeyCode::A) + (keysym - XK_A));
    }

    // Numbers
    if (keysym >= XK_0 && keysym <= XK_9) {
        return static_cast<KeyCode>(static_cast<int>(KeyCode::Num0) + (keysym - XK_0));
    }

    // Special keys
    switch (keysym) {
        case XK_space: return KeyCode::Space;
        case XK_Return: return KeyCode::Enter;
        case XK_BackSpace: return KeyCode::Backspace;
        case XK_Delete: return KeyCode::Delete;
        case XK_Tab: return KeyCode::Tab;
        case XK_Left: return KeyCode::Left;
        case XK_Right: return KeyCode::Right;
        case XK_Up: return KeyCode::Up;
        case XK_Down: return KeyCode::Down;
        case XK_Home: return KeyCode::Home;
        case XK_End: return KeyCode::End;
        case XK_Page_Up: return KeyCode::PageUp;
        case XK_Page_Down: return KeyCode::PageDown;
        case XK_Escape: return KeyCode::Escape;
        case XK_Control_L: return KeyCode::LeftControl;
        case XK_Control_R: return KeyCode::RightControl;
        case XK_Shift_L: return KeyCode::LeftShift;
        case XK_Shift_R: return KeyCode::RightShift;
        case XK_Alt_L: return KeyCode::LeftAlt;
        case XK_Alt_R: return KeyCode::RightAlt;
        default: return KeyCode::Unknown;
    }
}

WindowX11::WindowX11() {
    LOG_TRACE(LogCategory::PLATFORM, "WindowX11 constructor");
}

WindowX11::~WindowX11() {
    LOG_TRACE(LogCategory::PLATFORM, "WindowX11 destructor");
    destroy();
}

bool WindowX11::create(const WindowConfig& config) {
    LOG_INFO(LogCategory::PLATFORM, "Creating X11 window: %s (%dx%d)",
        config.title.c_str(), config.width, config.height);

    // Abrir conexión con el servidor X
    display_ = XOpenDisplay(nullptr);
    if (!display_) {
        LOG_ERROR(LogCategory::PLATFORM, "Failed to open X display");
        return false;
    }

    LOG_DEBUG(LogCategory::PLATFORM, "X11 display opened successfully");

    // Obtener la pantalla por defecto
    int screen = DefaultScreen(display_);
    Window root = RootWindow(display_, screen);

    // Crear la ventana
    width_ = config.width;
    height_ = config.height;

    window_ = XCreateSimpleWindow(
        display_,
        root,
        0, 0,  // x, y
        width_, height_,
        1,  // border width
        BlackPixel(display_, screen),
        BlackPixel(display_, screen)
    );

    if (!window_) {
        LOG_ERROR(LogCategory::PLATFORM, "Failed to create X11 window");
        XCloseDisplay(display_);
        display_ = nullptr;
        return false;
    }

    LOG_DEBUG(LogCategory::PLATFORM, "X11 window created: ID=%lu", window_);

    // Establecer el título
    XStoreName(display_, window_, config.title.c_str());

    // Configurar el protocolo de cierre de ventana
    wmDeleteMessage_ = XInternAtom(display_, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(display_, window_, &wmDeleteMessage_, 1);

    // Seleccionar los eventos que queremos recibir
    XSelectInput(display_, window_,
        ExposureMask | KeyPressMask | KeyReleaseMask |
        ButtonPressMask | ButtonReleaseMask |
        PointerMotionMask | StructureNotifyMask);

    // Mostrar la ventana
    XMapWindow(display_, window_);
    XFlush(display_);

    LOG_INFO(LogCategory::PLATFORM, "X11 window created and mapped successfully");

    return true;
}

void WindowX11::destroy() {
    if (window_) {
        LOG_INFO(LogCategory::PLATFORM, "Destroying X11 window");
        XDestroyWindow(display_, window_);
        window_ = 0;
    }

    if (display_) {
        LOG_DEBUG(LogCategory::PLATFORM, "Closing X11 display");
        XCloseDisplay(display_);
        display_ = nullptr;
    }
}

bool WindowX11::shouldClose() const {
    return shouldClose_;
}

void WindowX11::pollEvents() {
    if (!display_) {
        return;
    }

    // Procesar todos los eventos pendientes
    while (XPending(display_) > 0) {
        XEvent event;
        XNextEvent(display_, &event);

        switch (event.type) {
            case ClientMessage:
                if (static_cast<Atom>(event.xclient.data.l[0]) == wmDeleteMessage_) {
                    LOG_INFO(LogCategory::PLATFORM, "Window close requested");
                    shouldClose_ = true;
                }
                break;

            case ConfigureNotify:
                {
                    int newWidth = event.xconfigure.width;
                    int newHeight = event.xconfigure.height;
                    if (newWidth != width_ || newHeight != height_) {
                        width_ = newWidth;
                        height_ = newHeight;
                        LOG_DEBUG(LogCategory::PLATFORM, "Window resized: %dx%d", width_, height_);
                    }
                }
                break;

            case MapNotify:
                isMinimized_ = false;
                LOG_TRACE(LogCategory::PLATFORM, "Window mapped");
                break;

            case UnmapNotify:
                isMinimized_ = true;
                LOG_TRACE(LogCategory::PLATFORM, "Window unmapped");
                break;

            case 2:  // KeyPress (X11 macro value)
                {
                    KeySym keysym = XLookupKeysym(&event.xkey, 0);
                    KeyCode key = x11KeySymToKeyCode(keysym);

                    if (key != KeyCode::Unknown && inputCallback_) {
                        InputEvent inputEvent;
                        inputEvent.type = InputEvent::Type::KeyDown;
                        inputEvent.data.keyboard.key = key;
                        inputEvent.data.keyboard.shift = (event.xkey.state & ShiftMask) != 0;
                        inputEvent.data.keyboard.ctrl = (event.xkey.state & ControlMask) != 0;
                        inputEvent.data.keyboard.alt = (event.xkey.state & Mod1Mask) != 0;

                        inputCallback_(inputEvent);

                        // Also send character event for printable keys
                        char charBuffer[32];
                        KeySym tempKeysym;
                        int length = ::XLookupString(&event.xkey, charBuffer, sizeof(charBuffer) - 1, &tempKeysym, nullptr);
                        if (length > 0) {
                            charBuffer[length] = '\0';
                            InputEvent charEvent;
                            charEvent.type = InputEvent::Type::Character;
                            charEvent.data.character.codepoint = static_cast<uint32_t>(charBuffer[0]);
                            inputCallback_(charEvent);
                        }
                    }

                    LOG_TRACE(LogCategory::PLATFORM, "Key pressed: %d (keysym: %lu)", event.xkey.keycode, keysym);
                }
                break;

            case 3:  // KeyRelease (X11 macro value)
                LOG_TRACE(LogCategory::PLATFORM, "Key released: %d", event.xkey.keycode);
                break;

            case ButtonPress:
                LOG_TRACE(LogCategory::PLATFORM, "Mouse button pressed: %d", event.xbutton.button);
                break;

            case ButtonRelease:
                LOG_TRACE(LogCategory::PLATFORM, "Mouse button released: %d", event.xbutton.button);
                break;

            case MotionNotify:
                // Logging de mouse motion es muy verboso, solo en TRACE si es necesario
                // LOG_TRACE(LogCategory::PLATFORM, "Mouse moved: (%d, %d)", event.xmotion.x, event.xmotion.y);
                break;

            case Expose:
                LOG_TRACE(LogCategory::PLATFORM, "Expose event");
                break;

            default:
                break;
        }
    }
}

void WindowX11::getFramebufferSize(int& width, int& height) const {
    width = width_;
    height = height_;
}

bool WindowX11::isMinimized() const {
    return isMinimized_;
}

VkSurfaceKHR WindowX11::createVulkanSurface(VkInstance instance) {
    LOG_INFO(LogCategory::VULKAN, "Creating Vulkan surface for X11");

    VkXlibSurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    createInfo.dpy = display_;
    createInfo.window = window_;

    VkSurfaceKHR surface;
    VkResult result = vkCreateXlibSurfaceKHR(instance, &createInfo, nullptr, &surface);

    if (result != VK_SUCCESS) {
        LOG_ERROR(LogCategory::VULKAN, "Failed to create Vulkan X11 surface: %d", result);
        return VK_NULL_HANDLE;
    }

    LOG_INFO(LogCategory::VULKAN, "Vulkan X11 surface created successfully");
    return surface;
}

const char** WindowX11::getRequiredVulkanExtensions(uint32_t& count) const {
    static const char* extensions[] = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_XLIB_SURFACE_EXTENSION_NAME
    };

    count = 2;
    return extensions;
}

} // namespace phantom
