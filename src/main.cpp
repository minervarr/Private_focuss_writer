#include "platform/platform_interface.h"
#include "rendering/vulkan/vk_renderer.h"
#include "rendering/vulkan/vk_text_renderer.h"
#include "rendering/core/font_loader.h"
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

    // Load font
    LOG_INFO(phantom::LogCategory::INIT, "Loading font");
    phantom::FontLoader fontLoader;

    if (!fontLoader.loadFromFile("assets/fonts/default_mono.ttf", 48.0f)) {
        LOG_FATAL(phantom::LogCategory::INIT, "Failed to load font");
        renderer.cleanup();
        platform.cleanup();
        phantom::Logger::shutdown();
        return EXIT_FAILURE;
    }

    // Initialize text renderer
    LOG_INFO(phantom::LogCategory::INIT, "Initializing text renderer");
    phantom::VulkanTextRenderer textRenderer;

    if (!textRenderer.initialize(&renderer, renderer.getRenderPass(), fontLoader.getAtlas())) {
        LOG_FATAL(phantom::LogCategory::INIT, "Failed to initialize text renderer");
        renderer.cleanup();
        platform.cleanup();
        phantom::Logger::shutdown();
        return EXIT_FAILURE;
    }

    // Update projection matrix for text rendering
    textRenderer.updateProjection(windowConfig.width, windowConfig.height);

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

        // Render text: "Hello, Phantom!" centered
        float centerX = width / 2.0f - 200.0f;  // Approximate centering
        float centerY = height / 2.0f;
        textRenderer.renderText(renderer.getCurrentCommandBuffer(), "Hello, Phantom!", centerX, centerY);

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
    textRenderer.cleanup();
    renderer.cleanup();
    platform.cleanup();

    LOG_INFO(phantom::LogCategory::INIT, "Phantom Writer shutdown complete");
    phantom::Logger::shutdown();

    return EXIT_SUCCESS;
}
