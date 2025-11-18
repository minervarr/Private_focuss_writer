#include <android_native_app_glue.h>
#include <android/log.h>
#include "window_android.h"
#include "input_android.h"
#include "file_system_android.h"
#include "utils/logger.h"

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "PhantomWriter", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "PhantomWriter", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "PhantomWriter", __VA_ARGS__))

using namespace phantom;

struct AppState {
    android_app* app;
    WindowAndroid* window;
    bool isRunning;
};

static void handleAppCommand(android_app* app, int32_t cmd) {
    AppState* state = static_cast<AppState*>(app->userData);

    if (state && state->window) {
        state->window->handleAppCommand(cmd);
    }

    switch (cmd) {
        case APP_CMD_INIT_WINDOW:
            LOGI("APP_CMD_INIT_WINDOW");
            if (state && !state->isRunning) {
                state->isRunning = true;
            }
            break;

        case APP_CMD_TERM_WINDOW:
            LOGI("APP_CMD_TERM_WINDOW");
            state->isRunning = false;
            break;

        case APP_CMD_DESTROY:
            LOGI("APP_CMD_DESTROY");
            state->isRunning = false;
            break;

        default:
            break;
    }
}

static int32_t handleInputEvent(android_app* app, AInputEvent* event) {
    AppState* state = static_cast<AppState*>(app->userData);

    if (state && state->window) {
        return state->window->handleInputEvent(event);
    }

    return 0;
}

void android_main(android_app* androidApp) {
    LOGI("=== Phantom Writer v0.1.0 (Android) ===");

    AppState appState{};
    appState.app = androidApp;
    appState.isRunning = false;

    androidApp->userData = &appState;
    androidApp->onAppCmd = handleAppCommand;
    androidApp->onInputEvent = handleInputEvent;

    // Create platform context
    LOG_INFO(LogCategory::INIT, "Creating Android platform context");

    WindowAndroid* window = new WindowAndroid();
    InputAndroid* input = new InputAndroid();
    FileSystemAndroid* fileSystem = new FileSystemAndroid();

    // Set Android app reference
    window->setAndroidApp(androidApp);
    fileSystem->setAndroidApp(androidApp);

    appState.window = window;

    // Wait for window to be initialized
    LOGI("Waiting for window initialization...");
    while (!androidApp->window) {
        int events;
        android_poll_source* source;

        if (ALooper_pollAll(0, nullptr, &events, (void**)&source) >= 0) {
            if (source) {
                source->process(androidApp, source);
            }

            if (androidApp->destroyRequested) {
                LOGI("Destroy requested during initialization");
                goto cleanup;
            }
        }
    }

    // Create window
    WindowConfig config;
    config.title = "Phantom Writer";
    config.fullscreen = true;

    if (!window->create(config)) {
        LOGE("Failed to create window");
        goto cleanup;
    }

    LOGI("Entering main loop...");
    appState.isRunning = true;

    // Main loop
    while (!androidApp->destroyRequested && !window->shouldClose()) {
        int events;
        android_poll_source* source;

        // Process all pending events
        while (ALooper_pollAll(appState.isRunning ? 0 : -1, nullptr, &events, (void**)&source) >= 0) {
            if (source) {
                source->process(androidApp, source);
            }

            if (androidApp->destroyRequested) {
                LOGI("Destroy requested");
                goto cleanup;
            }
        }

        // Render frame when app is active
        if (appState.isRunning && !window->isMinimized()) {
            // TODO: Call renderer update here
            // renderer->update(deltaTime);
            // renderer->render();
        }
    }

cleanup:
    LOGI("Cleaning up...");

    window->destroy();
    delete window;
    delete input;
    delete fileSystem;

    LOGI("Shutdown complete");
}
