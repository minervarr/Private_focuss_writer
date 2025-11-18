#include "../platform_interface.h"
#include "window_win32.h"
#include "input_win32.h"
#include "file_system_windows.h"
#include "utils/logger.h"

namespace phantom {

PlatformContext createPlatformContext() {
    LOG_INFO(LogCategory::PLATFORM, "Creating Windows platform context");

    PlatformContext context;
    context.window = new WindowWin32();
    context.input = new InputWin32();
    context.fileSystem = new FileSystemWindows();

    LOG_DEBUG(LogCategory::PLATFORM, "Windows platform context created");

    return context;
}

} // namespace phantom
