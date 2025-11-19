#include "../platform_interface.h"

// Conditional window implementation
#ifdef PHANTOM_USE_SOKOL_VULKAN
    #include "window_sokol_vulkan.h"
#else
    #include "window_win32.h"
#endif

#include "input_win32.h"
#include "file_system_windows.h"
#include "utils/logger.h"

namespace phantom {

PlatformContext createPlatformContext() {
    #ifdef PHANTOM_USE_SOKOL_VULKAN
        LOG_INFO(LogCategory::PLATFORM, "Creating Windows platform context with Sokol+Vulkan");
    #else
        LOG_INFO(LogCategory::PLATFORM, "Creating Windows platform context with pure Win32+Vulkan");
    #endif

    PlatformContext context;

    #ifdef PHANTOM_USE_SOKOL_VULKAN
        context.window = new WindowSokolVulkan();
    #else
        context.window = new WindowWin32();
    #endif

    context.input = new InputWin32();
    context.fileSystem = new FileSystemWindows();

    LOG_DEBUG(LogCategory::PLATFORM, "Windows platform context created");

    return context;
}

} // namespace phantom
