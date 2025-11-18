#include "../platform_interface.h"
#include "window_x11.h"
#include "input_x11.h"
#include "file_system_linux.h"
#include "utils/logger.h"

namespace phantom {

PlatformContext createPlatformContext() {
    LOG_INFO(LogCategory::PLATFORM, "Creating Linux platform context");

    PlatformContext context;
    context.window = new WindowX11();
    context.input = new InputX11();
    context.fileSystem = new FileSystemLinux();

    LOG_DEBUG(LogCategory::PLATFORM, "Linux platform context created");

    return context;
}

} // namespace phantom
