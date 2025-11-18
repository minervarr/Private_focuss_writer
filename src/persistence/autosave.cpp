#include "autosave.h"
#include "swap_file.h"
#include "core/buffer.h"
#include "core/cursor.h"
#include "utils/logger.h"

#include <chrono>

namespace phantom {

Autosave::Autosave(SwapFile* swapFile, const TextBuffer& buffer, const Cursor& cursor)
    : swapFile_(swapFile)
    , buffer_(buffer)
    , cursor_(cursor)
    , running_(false)
    , shouldExit_(false)
    , isDirty_(false)
{
    LOG_DEBUG(LogCategory::PERSISTENCE, "Autosave created");
}

Autosave::~Autosave() {
    LOG_DEBUG(LogCategory::PERSISTENCE, "Autosave destructor called");
    stop();
}

void Autosave::start() {
    if (running_.load()) {
        LOG_WARN(LogCategory::PERSISTENCE, "Autosave already running");
        return;
    }

    LOG_INFO(LogCategory::PERSISTENCE, "Starting autosave thread (interval: %.1fs)", AUTOSAVE_INTERVAL);

    running_.store(true);
    shouldExit_.store(false);
    isDirty_.store(false);

    autosaveThread_ = std::thread(&Autosave::autosaveLoop, this);
}

void Autosave::stop() {
    if (!running_.load()) {
        return;
    }

    LOG_INFO(LogCategory::PERSISTENCE, "Stopping autosave thread");

    shouldExit_.store(true);
    cv_.notify_one();

    if (autosaveThread_.joinable()) {
        autosaveThread_.join();
    }

    running_.store(false);
    LOG_DEBUG(LogCategory::PERSISTENCE, "Autosave thread stopped");
}

void Autosave::saveNow() {
    LOG_DEBUG(LogCategory::PERSISTENCE, "Manual save triggered");

    if (swapFile_->write(buffer_, cursor_)) {
        isDirty_.store(false);
        LOG_INFO(LogCategory::PERSISTENCE, "Manual save successful");
    } else {
        LOG_ERROR(LogCategory::PERSISTENCE, "Manual save failed");
    }
}

void Autosave::markDirty() {
    isDirty_.store(true);
}

void Autosave::autosaveLoop() {
    LOG_DEBUG(LogCategory::PERSISTENCE, "Autosave thread started");

    while (!shouldExit_.load()) {
        std::unique_lock<std::mutex> lock(mutex_);

        // Wait for interval or exit signal
        cv_.wait_for(lock, std::chrono::milliseconds(static_cast<int>(AUTOSAVE_INTERVAL * 1000)),
                     [this]() { return shouldExit_.load(); });

        if (shouldExit_.load()) {
            break;
        }

        // Check if buffer has been modified
        if (isDirty_.load()) {
            LOG_TRACE(LogCategory::PERSISTENCE, "Autosaving...");

            if (swapFile_->write(buffer_, cursor_)) {
                isDirty_.store(false);
                LOG_DEBUG(LogCategory::PERSISTENCE, "Autosave successful");
            } else {
                LOG_ERROR(LogCategory::PERSISTENCE, "Autosave failed");
            }
        }
    }

    LOG_DEBUG(LogCategory::PERSISTENCE, "Autosave thread exiting");
}

} // namespace phantom
