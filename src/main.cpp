#include "platform/platform_interface.h"
#include "rendering/vulkan/vk_renderer.h"
#include "utils/logger.h"
#include "phantom_writer/version.h"

#include <cstdlib>

int main() {
    // Initialize logger
    phantom::Logger::init("phantom_writer.log");

    LOG_INFO(phantom::LogCategory::INIT, "=== Phantom Writer v%d.%d.%d ===",
        PHANTOM_WRITER_VERSION_MAJOR,
        PHANTOM_WRITER_VERSION_MINOR,
        PHANTOM_WRITER_VERSION_PATCH);

#ifdef DEBUG_BUILD
    LOG_INFO(phantom::LogCategory::INIT, "Build: DEBUG");
#else
    LOG_INFO(phantom::LogCategory::INIT, "Build: RELEASE");
#endif

#ifdef __linux__
    LOG_INFO(phantom::LogCategory::INIT, "Platform: Linux");
#elif _WIN32
    LOG_INFO(phantom::LogCategory::INIT, "Platform: Windows");
#elif __ANDROID__
    LOG_INFO(phantom::LogCategory::INIT, "Platform: Android");
#else
    LOG_INFO(phantom::LogCategory::INIT, "Platform: Unknown");
#endif

    // Create platform context
    LOG_INFO(phantom::LogCategory::INIT, "Creating platform context");
    phantom::PlatformContext platform = phantom::createPlatformContext();

    if (!platform.window) {
        LOG_FATAL(phantom::LogCategory::INIT, "Failed to create platform window");
        phantom::Logger::shutdown();
        return EXIT_FAILURE;
    }

    // Create window
    phantom::WindowConfig windowConfig;
    windowConfig.title = "Phantom Writer v" PHANTOM_WRITER_VERSION;
    windowConfig.width = 1280;
    windowConfig.height = 720;
    windowConfig.resizable = true;
    windowConfig.decorated = true;

    LOG_INFO(phantom::LogCategory::INIT, "Creating window: %s (%dx%d)",
        windowConfig.title.c_str(), windowConfig.width, windowConfig.height);

    if (!platform.window->create(windowConfig)) {
        LOG_FATAL(phantom::LogCategory::INIT, "Failed to create window");
        platform.cleanup();
        phantom::Logger::shutdown();
        return EXIT_FAILURE;
    }

    // Initialize Vulkan renderer
    LOG_INFO(phantom::LogCategory::INIT, "Initializing Vulkan renderer");
    phantom::VulkanRenderer renderer;

    if (!renderer.initialize(platform.window)) {
        LOG_FATAL(phantom::LogCategory::INIT, "Failed to initialize Vulkan renderer");
        platform.cleanup();
        phantom::Logger::shutdown();
        return EXIT_FAILURE;
    }

    LOG_INFO(phantom::LogCategory::INIT, "Phantom Writer started successfully");
    LOG_INFO(phantom::LogCategory::INIT, "Entering main loop");

    // Main loop
    uint64_t frameCount = 0;
    while (!platform.window->shouldClose()) {
        // Poll events
        platform.window->pollEvents();

        // Skip rendering if minimized
        int width, height;
        platform.window->getFramebufferSize(width, height);
        if (width == 0 || height == 0 || platform.window->isMinimized()) {
            continue;
        }

        // Render frame
        renderer.beginFrame();
        // Drawing commands would go here in future phases
        renderer.endFrame();

        frameCount++;
        if (frameCount % 600 == 0) {  // Log every 10 seconds at 60 FPS
            LOG_DEBUG(phantom::LogCategory::RENDER, "Rendered %lu frames", frameCount);
        }
    }

    LOG_INFO(phantom::LogCategory::INIT, "Exiting main loop");
    LOG_INFO(phantom::LogCategory::INIT, "Total frames rendered: %lu", frameCount);

    // Cleanup
    LOG_INFO(phantom::LogCategory::INIT, "Cleaning up resources");
    renderer.cleanup();
    platform.cleanup();

    LOG_INFO(phantom::LogCategory::INIT, "Phantom Writer shutdown complete");
    phantom::Logger::shutdown();

    return EXIT_SUCCESS;
}
