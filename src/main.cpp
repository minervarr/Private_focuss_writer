#include "platform/platform_interface.h"
#include "platform/linux/window_x11.h"
#include "rendering/vulkan/vk_renderer.h"
#include "rendering/vulkan/vk_text_renderer.h"
#include "rendering/core/font_loader.h"
#include "core/editor_state.h"
#include "persistence/swap_file.h"
#include "ui/revision_mode.h"
#include "ui/confirmation_dialog.h"
#include "utils/logger.h"
#include "phantom_writer/version.h"

#include <cstdlib>
#include <chrono>

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

    // Create editor state (buffer + cursor + persistence)
    std::string filePath = ""; // TODO: Get from command line args
    phantom::EditorState editorState(filePath);

    // Check for crash recovery
    if (editorState.getSwapFile()->exists()) {
        if (editorState.getSwapFile()->isNewerThanOriginal()) {
            LOG_WARN(phantom::LogCategory::PERSISTENCE, "Swap file detected - possible crash recovery");
            LOG_INFO(phantom::LogCategory::PERSISTENCE, "Attempting to load from swap file");
            if (editorState.loadFromSwapFile()) {
                LOG_INFO(phantom::LogCategory::PERSISTENCE, "Successfully recovered from swap file");
            } else {
                LOG_ERROR(phantom::LogCategory::PERSISTENCE, "Failed to recover from swap file");
            }
        } else {
            LOG_INFO(phantom::LogCategory::PERSISTENCE, "Removing old swap file");
            editorState.getSwapFile()->remove();
        }
    }

    // Start autosave thread
    editorState.startAutosave();

    // Setup input callback to handle keyboard events
    auto* x11Window = dynamic_cast<phantom::WindowX11*>(platform.window);
    if (x11Window) {
        x11Window->setInputCallback([&editorState](const phantom::InputEvent& event) {
            if (event.type == phantom::InputEvent::Type::Character) {
                // If confirmation dialog is active, route input to it
                if (editorState.getConfirmationDialog()->isActive()) {
                    char ch = static_cast<char>(event.data.character.codepoint);
                    editorState.getConfirmationDialog()->processInput(ch);
                    LOG_TRACE(phantom::LogCategory::UI, "Confirmation input: '%c'", ch);

                    // Check if confirmation succeeded
                    if (editorState.getConfirmationDialog()->isConfirmed()) {
                        editorState.getRevisionMode()->activate();
                        LOG_INFO(phantom::LogCategory::UI, "Revision mode activated");
                    }
                    return;
                }

                // Insert printable character (only if not in confirmation dialog)
                char ch = static_cast<char>(event.data.character.codepoint);
                editorState.insertChar(ch);
                LOG_TRACE(phantom::LogCategory::INPUT, "Character inserted: '%c'", ch);
            }
            else if (event.type == phantom::InputEvent::Type::KeyDown) {
                const auto& kbd = event.data.keyboard;

                // Handle Ctrl+R (activate revision mode)
                if (kbd.ctrl && kbd.key == phantom::KeyCode::R) {
                    if (!editorState.getRevisionMode()->isActive()) {
                        editorState.getConfirmationDialog()->startConfirmation();
                        LOG_INFO(phantom::LogCategory::UI, "Confirmation dialog shown");
                    }
                    return;
                }

                // Handle Ctrl+S (manual save)
                if (kbd.ctrl && kbd.key == phantom::KeyCode::S) {
                    editorState.saveNow();
                    LOG_INFO(phantom::LogCategory::PERSISTENCE, "Manual save triggered (Ctrl+S)");
                    return;
                }

                // Handle special keys
                switch (kbd.key) {
                    case phantom::KeyCode::Escape:
                        // Deactivate revision mode or cancel confirmation dialog
                        if (editorState.getRevisionMode()->isActive()) {
                            editorState.getRevisionMode()->deactivate();
                            LOG_INFO(phantom::LogCategory::UI, "Revision mode deactivated");
                        } else if (editorState.getConfirmationDialog()->isActive()) {
                            editorState.getConfirmationDialog()->cancel();
                            LOG_INFO(phantom::LogCategory::UI, "Confirmation dialog cancelled");
                        }
                        break;

                    case phantom::KeyCode::Backspace:
                        // Handle backspace in confirmation dialog
                        if (editorState.getConfirmationDialog()->isActive()) {
                            editorState.getConfirmationDialog()->processBackspace();
                            LOG_TRACE(phantom::LogCategory::UI, "Backspace in confirmation dialog");
                        } else {
                            editorState.deleteChar();
                            LOG_TRACE(phantom::LogCategory::INPUT, "Backspace pressed");
                        }
                        break;

                    case phantom::KeyCode::Enter:
                        editorState.insertChar('\n');
                        LOG_TRACE(phantom::LogCategory::INPUT, "Enter pressed");
                        break;

                    case phantom::KeyCode::Left:
                        editorState.getCursor().moveLeft(editorState.getBuffer());
                        editorState.getOpacityManager().onActivity();
                        LOG_TRACE(phantom::LogCategory::INPUT, "Left arrow pressed");
                        break;

                    case phantom::KeyCode::Right:
                        editorState.getCursor().moveRight(editorState.getBuffer());
                        editorState.getOpacityManager().onActivity();
                        LOG_TRACE(phantom::LogCategory::INPUT, "Right arrow pressed");
                        break;

                    case phantom::KeyCode::Up:
                        editorState.getCursor().moveUp(editorState.getBuffer());
                        editorState.getOpacityManager().onActivity();
                        LOG_TRACE(phantom::LogCategory::INPUT, "Up arrow pressed");
                        break;

                    case phantom::KeyCode::Down:
                        editorState.getCursor().moveDown(editorState.getBuffer());
                        editorState.getOpacityManager().onActivity();
                        LOG_TRACE(phantom::LogCategory::INPUT, "Down arrow pressed");
                        break;

                    case phantom::KeyCode::Home:
                        editorState.getCursor().moveToLineStart(editorState.getBuffer());
                        editorState.getOpacityManager().onActivity();
                        LOG_TRACE(phantom::LogCategory::INPUT, "Home pressed");
                        break;

                    case phantom::KeyCode::End:
                        editorState.getCursor().moveToLineEnd(editorState.getBuffer());
                        editorState.getOpacityManager().onActivity();
                        LOG_TRACE(phantom::LogCategory::INPUT, "End pressed");
                        break;

                    default:
                        break;
                }
            }
        });
    }

    LOG_INFO(phantom::LogCategory::INIT, "Phantom Writer started successfully");
    LOG_INFO(phantom::LogCategory::INIT, "Entering main loop");

    // Main loop with delta time tracking
    uint64_t frameCount = 0;
    auto lastFrameTime = std::chrono::high_resolution_clock::now();

    while (!platform.window->shouldClose()) {
        // Calculate delta time
        auto currentFrameTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;
        float deltaSeconds = deltaTime.count();

        // Update opacity manager
        editorState.getOpacityManager().update(deltaSeconds);

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

        // Render buffer content
        std::string bufferText = editorState.getBuffer().getText();
        if (bufferText.empty()) {
            bufferText = "";  // Show empty if nothing typed yet
        }

        // Check if revision mode is active
        bool revisionModeActive = editorState.getRevisionMode()->isActive();

        // Calculate opacity - full opacity in revision mode, otherwise based on typing state
        float opacity;
        if (revisionModeActive) {
            opacity = 1.0f;  // Full opacity in revision mode
        } else {
            opacity = editorState.getOpacityManager().isIdle() ? 1.0f :
                      editorState.getOpacityManager().getPreviousLinesOpacity();
        }

        // Disable fragmentation in revision mode
        bool disableFragmentation = revisionModeActive;

        // Render at top-left with some padding
        float textX = 20.0f;
        float textY = 50.0f;
        textRenderer.renderText(renderer.getCurrentCommandBuffer(), bufferText, textX, textY, 1.0f, opacity, disableFragmentation);

        // Render UI overlays
        // Confirmation dialog prompt at bottom
        if (editorState.getConfirmationDialog()->isActive()) {
            std::string prompt = editorState.getConfirmationDialog()->getPromptMessage();
            float promptX = 20.0f;
            float promptY = height - 40.0f;
            textRenderer.renderText(renderer.getCurrentCommandBuffer(), prompt, promptX, promptY, 0.8f, 1.0f, true);
        }

        // Revision mode indicator at top-right (red warning)
        if (revisionModeActive) {
            std::string indicator = "[ REVISION MODE - TEXT VISIBLE ]";
            float indicatorX = width - 600.0f;  // Approximate position
            float indicatorY = 20.0f;
            textRenderer.renderText(renderer.getCurrentCommandBuffer(), indicator, indicatorX, indicatorY, 0.7f, 1.0f, true);
        }

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

    // Stop autosave and remove swap file on clean exit
    editorState.stopAutosave();
    if (editorState.getSwapFile()->exists()) {
        LOG_INFO(phantom::LogCategory::PERSISTENCE, "Removing swap file on clean exit");
        editorState.getSwapFile()->remove();
    }

    textRenderer.cleanup();
    renderer.cleanup();
    platform.cleanup();

    LOG_INFO(phantom::LogCategory::INIT, "Phantom Writer shutdown complete");
    phantom::Logger::shutdown();

    return EXIT_SUCCESS;
}
