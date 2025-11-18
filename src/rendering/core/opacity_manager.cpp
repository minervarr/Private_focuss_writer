#include "opacity_manager.h"
#include "utils/logger.h"

namespace phantom {

OpacityManager::OpacityManager()
    : timeSinceLastActivity_(0.0f)
    , isIdle_(true) // Start in idle mode
{
    LOG_TRACE(LogCategory::RENDER, "OpacityManager created");
}

OpacityManager::~OpacityManager() {
    LOG_TRACE(LogCategory::RENDER, "OpacityManager destroyed");
}

void OpacityManager::update(float deltaTime) {
    timeSinceLastActivity_ += deltaTime;

    // Check if we've crossed the idle threshold
    bool wasIdle = isIdle_;
    isIdle_ = (timeSinceLastActivity_ >= idleThreshold_);

    // Log state transitions
    if (!wasIdle && isIdle_) {
        LOG_DEBUG(LogCategory::RENDER, "Entering idle mode - full opacity restored");
    }
}

void OpacityManager::onActivity() {
    bool wasIdle = isIdle_;

    timeSinceLastActivity_ = 0.0f;
    isIdle_ = false;

    if (wasIdle) {
        LOG_DEBUG(LogCategory::RENDER, "Activity detected - entering typing mode");
    }
}

float OpacityManager::getOpacityForLine(size_t lineNumber, size_t currentLine) const {
    // If idle, everything is fully opaque
    if (isIdle_) {
        return fullOpacity_;
    }

    // Current line is always fully opaque
    if (lineNumber == currentLine) {
        return fullOpacity_;
    }

    // Previous lines have reduced opacity when typing
    if (lineNumber < currentLine) {
        return previousLinesOpacity_;
    }

    // Lines after current line are fully opaque
    return fullOpacity_;
}

} // namespace phantom
