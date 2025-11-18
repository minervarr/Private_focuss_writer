#include "revision_mode.h"
#include "utils/logger.h"

namespace phantom {

RevisionMode::RevisionMode()
    : active_(false)
{
    LOG_DEBUG(LogCategory::UI, "RevisionMode created");
}

RevisionMode::~RevisionMode() {
    LOG_TRACE(LogCategory::UI, "RevisionMode destroyed");
}

void RevisionMode::activate() {
    if (active_) {
        LOG_WARN(LogCategory::UI, "Revision mode already active");
        return;
    }

    active_ = true;
    LOG_INFO(LogCategory::UI, "Revision mode ACTIVATED - text fully visible");
}

void RevisionMode::deactivate() {
    if (!active_) {
        return;
    }

    active_ = false;
    LOG_INFO(LogCategory::UI, "Revision mode DEACTIVATED - returning to normal mode");
}

} // namespace phantom
