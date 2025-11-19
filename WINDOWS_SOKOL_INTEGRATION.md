# Windows Sokol+Vulkan Integration

## Overview

Phantom Writer now supports two window/input backends on Windows:

1. **Sokol + Vulkan** (Default, Recommended)
   - Modern, cross-platform window/input abstraction
   - Cleaner codebase, easier to maintain
   - Better event handling
   - More portable architecture

2. **Pure Win32 + Vulkan** (Legacy)
   - Direct Win32 API usage
   - Lower-level control
   - Can be selected via CMake option

---

## Architecture

### Sokol Integration

The Sokol integration provides a modern window abstraction layer that simplifies Windows development:

```
┌─────────────────────────────────────────┐
│         Phantom Writer Core             │
│   (Editor, Rendering, Persistence)      │
└────────────────┬────────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────────┐
│      WindowSokolVulkan Wrapper          │
│  - Event handling                       │
│  - Vulkan surface creation              │
│  - Input mapping                        │
└────────────────┬────────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────────┐
│         Sokol App Library               │
│  (sokol_app.h - Window management)      │
└────────────────┬────────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────────┐
│            Win32 API                    │
│  (HWND, Messages, Input)                │
└─────────────────────────────────────────┘
```

### Key Components

#### 1. WindowSokolVulkan (`src/platform/windows/window_sokol_vulkan.h/cpp`)

Implements the `IPlatformWindow` interface using Sokol:

- **Event Handling**: Converts Sokol events to Phantom input events
- **Vulkan Integration**: Creates Vulkan surface from Sokol window handle
- **Input Mapping**: Maps Sokol key codes to internal KeyCode enum
- **Window Management**: Fullscreen, minimize, resize handling

#### 2. Platform Context (`src/platform/windows/platform_windows.cpp`)

Conditionally creates the appropriate window implementation:

```cpp
#ifdef PHANTOM_USE_SOKOL_VULKAN
    context.window = new WindowSokolVulkan();
#else
    context.window = new WindowWin32();
#endif
```

#### 3. CMake Configuration (`src/platform/windows/CMakeLists.txt`)

Provides build-time option to select backend:

```cmake
option(USE_SOKOL_VULKAN "Use Sokol with Vulkan backend for Windows" ON)
```

---

## Building on Windows

### Prerequisites

1. **CMake** (3.20+)
   - Download: https://cmake.org/download/
   - Add to PATH during installation

2. **Visual Studio 2019 or newer** (or MinGW-w64)
   - Visual Studio: Install "Desktop development with C++" workload
   - MinGW: https://www.mingw-w64.org/

3. **Vulkan SDK**
   - Download: https://vulkan.lunarg.com/sdk/home#windows
   - Install and ensure `VULKAN_SDK` environment variable is set
   - Includes `glslc` shader compiler

### Build with Sokol+Vulkan (Default)

```batch
# Simple build
build_windows.bat

# Specific build type
build_windows.bat Release

# With Visual Studio 2022
build_windows.bat Debug
```

### Build with Pure Win32+Vulkan

```batch
# Disable Sokol, use Win32
build_windows.bat --no-sokol

# Release build without Sokol
build_windows.bat --no-sokol Release
```

### Manual CMake Configuration

```batch
mkdir build
cd build

# With Sokol (default)
cmake -G "Visual Studio 17 2022" -A x64 -DUSE_SOKOL_VULKAN=ON ..

# Without Sokol (pure Win32)
cmake -G "Visual Studio 17 2022" -A x64 -DUSE_SOKOL_VULKAN=OFF ..

# Build
cmake --build . --config Debug
```

---

## Running the Application

### From Build Directory

**Visual Studio Build**:
```batch
cd build\bin\Debug
phantom-writer.exe
```

**MinGW Build**:
```batch
cd build\bin
phantom-writer.exe
```

### Assets and Shaders

The build process automatically:
1. Compiles GLSL shaders to SPIR-V (`.spv` files)
2. Copies compiled shaders to `build/bin/shaders/`
3. Copies fonts to `build/bin/assets/fonts/`

Required files:
```
build/bin/
├── phantom-writer.exe
├── shaders/
│   ├── text_vert.spv
│   └── text_frag.spv
└── assets/
    └── fonts/
        └── default_mono.ttf
```

---

## Shader Compilation

### Automatic (via CMake)

Shaders are compiled automatically during the build process.

### Manual Compilation

```batch
compile_shaders_windows.bat
```

This script:
- Detects `glslc` or `glslangValidator`
- Compiles `shaders/text.vert` → `build/shaders/text_vert.spv`
- Compiles `shaders/text.frag` → `build/shaders/text_frag.spv`
- Copies to `build/bin/shaders/` if directory exists

---

## Sokol Integration Details

### Event Handling

Sokol events are converted to Phantom input events in `WindowSokolVulkan::handleSokolEvent()`:

| Sokol Event | Phantom Event | Description |
|-------------|---------------|-------------|
| `SAPP_EVENTTYPE_KEY_DOWN` | `InputEvent::Type::KeyDown` | Keyboard key pressed |
| `SAPP_EVENTTYPE_KEY_UP` | `InputEvent::Type::KeyUp` | Keyboard key released |
| `SAPP_EVENTTYPE_CHAR` | `InputEvent::Type::Character` | Character input (for text) |
| `SAPP_EVENTTYPE_MOUSE_DOWN` | `InputEvent::Type::MousePress` | Mouse button pressed |
| `SAPP_EVENTTYPE_MOUSE_UP` | `InputEvent::Type::MouseRelease` | Mouse button released |
| `SAPP_EVENTTYPE_MOUSE_MOVE` | `InputEvent::Type::MouseMove` | Mouse movement |
| `SAPP_EVENTTYPE_MOUSE_SCROLL` | `InputEvent::Type::MouseScroll` | Mouse wheel scroll |
| `SAPP_EVENTTYPE_RESIZED` | (internal) | Window resized |
| `SAPP_EVENTTYPE_QUIT_REQUESTED` | (internal) | Window close requested |

### Key Mapping

All standard keys are mapped from Sokol to Phantom KeyCode enum:

```cpp
KeyCode WindowSokolVulkan::sokolKeyToKeyCode(int sapp_keycode) const {
    switch (sapp_keycode) {
        case SAPP_KEYCODE_A: return KeyCode::A;
        case SAPP_KEYCODE_ESCAPE: return KeyCode::Escape;
        case SAPP_KEYCODE_ENTER: return KeyCode::Enter;
        // ... (full mapping in window_sokol_vulkan.cpp:68)
    }
}
```

Supported keys:
- A-Z letters
- 0-9 numbers
- Arrow keys (Up, Down, Left, Right)
- Function keys (F1-F12)
- Modifiers (Shift, Ctrl, Alt)
- Special keys (Enter, Backspace, Delete, Tab, Escape, Space)
- Navigation (Home, End, PageUp, PageDown)

### Vulkan Surface Creation

WindowSokolVulkan creates Vulkan surfaces using native Win32 handles:

```cpp
VkSurfaceKHR WindowSokolVulkan::createVulkanSurface(VkInstance instance) {
    HWND hwnd = (HWND)sapp_win32_get_hwnd();
    HINSTANCE hinstance = GetModuleHandle(nullptr);

    VkWin32SurfaceCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hinstance = hinstance;
    createInfo.hwnd = hwnd;

    vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &vulkanSurface_);
    return vulkanSurface_;
}
```

Required Vulkan extensions:
- `VK_KHR_surface`
- `VK_KHR_win32_surface`

---

## Troubleshooting

### Build Issues

**Error: "VULKAN_SDK not found"**
```
Solution: Install Vulkan SDK and restart terminal
Download: https://vulkan.lunarg.com/sdk/home#windows
```

**Error: "No compiler found"**
```
Solution: Install Visual Studio 2019+ with C++ workload
Or install MinGW-w64: https://www.mingw-w64.org/
```

**Error: "cmake not found"**
```
Solution: Install CMake and add to PATH
Download: https://cmake.org/download/
```

### Runtime Issues

**Error: "Failed to create Vulkan instance"**
```
Solution: Ensure Vulkan drivers are installed
- NVIDIA: GeForce Game Ready Driver
- AMD: Adrenalin Software
- Intel: Graphics drivers from Intel website
```

**Error: "Shader not found"**
```
Solution: Ensure shaders are compiled and in correct location
Run: compile_shaders_windows.bat
Check: build\bin\shaders\*.spv files exist
```

**Error: "Font not found"**
```
Solution: Ensure assets are copied
Check: build\bin\assets\fonts\default_mono.ttf exists
```

### Switching Backends

To switch between Sokol and Win32 backends:

1. **Clean build directory**:
   ```batch
   rd /s /q build
   ```

2. **Reconfigure with desired backend**:
   ```batch
   # Sokol backend
   build_windows.bat

   # Win32 backend
   build_windows.bat --no-sokol
   ```

---

## Performance Considerations

### Sokol vs Win32

| Aspect | Sokol + Vulkan | Win32 + Vulkan |
|--------|----------------|----------------|
| **Code complexity** | Lower | Higher |
| **Maintainability** | Better | More difficult |
| **Performance** | Excellent | Excellent |
| **Portability** | High (easy to add Linux/macOS) | Low (Windows-only) |
| **Control** | Abstracted | Full low-level access |
| **Event handling** | Simplified | Manual message loop |

**Recommendation**: Use Sokol+Vulkan unless you need specific Win32 features.

---

## Files Added/Modified

### New Files

```
third_party/sokol/
├── sokol_app.h          # Sokol application/window library (512KB)
├── sokol_gfx.h          # Sokol graphics library (1MB)
└── sokol_glue.h         # Sokol glue code (5.5KB)

src/platform/windows/
├── window_sokol_vulkan.h    # Sokol window wrapper header
└── window_sokol_vulkan.cpp  # Sokol window implementation (387 lines)

build_windows.bat                # Windows build script
compile_shaders_windows.bat      # Shader compilation script
WINDOWS_SOKOL_INTEGRATION.md     # This documentation
```

### Modified Files

```
src/platform/windows/platform_windows.cpp  # Conditional window creation
src/platform/windows/CMakeLists.txt        # Added USE_SOKOL_VULKAN option
```

---

## Future Enhancements

Potential improvements for Sokol integration:

1. **Linux Support**: Extend Sokol wrapper to Linux with X11/Wayland
2. **macOS Support**: Add macOS Metal backend via Sokol
3. **Mobile Support**: Android/iOS via Sokol mobile backends
4. **Gamepad Input**: Add gamepad support through Sokol
5. **High DPI**: Better high-DPI scaling support
6. **Multi-window**: Support for multiple editor windows

---

## References

- **Sokol Headers**: https://github.com/floooh/sokol
- **Vulkan SDK**: https://vulkan.lunarg.com/
- **Win32 API**: https://docs.microsoft.com/en-us/windows/win32/

---

## License

Sokol headers are public domain/MIT licensed.
See: https://github.com/floooh/sokol/blob/master/LICENSE

---

**Document Version**: 1.0
**Last Updated**: 2025-11-19
**Author**: Autonomous Integration System
