#include "editor_state.h"
#include "persistence/swap_file.h"
#include "persistence/autosave.h"
#include "utils/logger.h"

namespace phantom {

EditorState::EditorState(const std::string& filePath)
{
    LOG_DEBUG(LogCategory::INIT, "EditorState created with file: %s",
              filePath.empty() ? "(untitled)" : filePath.c_str());

    // Create swap file manager
    swapFile_ = std::make_unique<SwapFile>(filePath);

    // Create autosave manager (but don't start it yet)
    autosave_ = std::make_unique<Autosave>(swapFile_.get(), buffer_, cursor_);
}

EditorState::~EditorState() {
    LOG_DEBUG(LogCategory::INIT, "EditorState destructor");

    // Stop autosave if running
    if (autosave_) {
        autosave_->stop();
    }
}

void EditorState::startAutosave() {
    if (autosave_) {
        autosave_->start();
        LOG_INFO(LogCategory::PERSISTENCE, "Autosave started");
    }
}

void EditorState::stopAutosave() {
    if (autosave_) {
        autosave_->stop();
        LOG_INFO(LogCategory::PERSISTENCE, "Autosave stopped");
    }
}

void EditorState::saveNow() {
    if (autosave_) {
        autosave_->saveNow();
    }
}

bool EditorState::loadFromSwapFile() {
    if (swapFile_ && swapFile_->exists()) {
        LOG_INFO(LogCategory::PERSISTENCE, "Loading from swap file");
        return swapFile_->read(buffer_, cursor_);
    }
    return false;
}

void EditorState::markDirty() {
    if (autosave_) {
        autosave_->markDirty();
    }
}

} // namespace phantom
