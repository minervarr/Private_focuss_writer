#include "../platform_interface.h"
#include "window_android.h"
#include "input_android.h"
#include "file_system_android.h"
#include "utils/logger.h"

namespace phantom {

PlatformContext createPlatformContext() {
    LOG_INFO(LogCategory::PLATFORM, "Creating Android platform context");

    PlatformContext context;
    context.window = new WindowAndroid();
    context.input = new InputAndroid();
    context.fileSystem = new FileSystemAndroid();

    LOG_DEBUG(LogCategory::PLATFORM, "Android platform context created");

    return context;
}

} // namespace phantom
