#ifndef PHANTOM_EDITOR_STATE_H
#define PHANTOM_EDITOR_STATE_H

#include "buffer.h"
#include "cursor.h"
#include "rendering/core/opacity_manager.h"
#include <memory>
#include <string>

namespace phantom {

class SwapFile;
class Autosave;
class RevisionMode;
class ConfirmationDialog;

// Simple editor state that holds buffer, cursor, opacity manager, persistence, and UI state
class EditorState {
public:
    EditorState(const std::string& filePath = "");
    ~EditorState();

    TextBuffer& getBuffer() { return buffer_; }
    const TextBuffer& getBuffer() const { return buffer_; }

    Cursor& getCursor() { return cursor_; }
    const Cursor& getCursor() const { return cursor_; }

    OpacityManager& getOpacityManager() { return opacityManager_; }
    const OpacityManager& getOpacityManager() const { return opacityManager_; }

    SwapFile* getSwapFile() { return swapFile_.get(); }
    Autosave* getAutosave() { return autosave_.get(); }

    RevisionMode* getRevisionMode() { return revisionMode_.get(); }
    ConfirmationDialog* getConfirmationDialog() { return confirmationDialog_.get(); }

    // Persistence
    void startAutosave();
    void stopAutosave();
    void saveNow();
    bool loadFromSwapFile();

    // Convenience methods
    void insertChar(char ch) {
        buffer_.insert(cursor_.getPosition(), ch);
        cursor_.setPosition(cursor_.getPosition() + 1);
        opacityManager_.onActivity(); // Notify activity
        markDirty();
    }

    void deleteChar() {
        if (cursor_.getPosition() > 0) {
            size_t pos = cursor_.getPosition() - 1;
            buffer_.erase(pos, 1);
            cursor_.setPosition(pos);
            opacityManager_.onActivity(); // Notify activity
            markDirty();
        }
    }

    void moveCursor(size_t newPosition) {
        cursor_.setPosition(newPosition);
        opacityManager_.onActivity(); // Notify activity
    }

private:
    void markDirty();

    TextBuffer buffer_;
    Cursor cursor_;
    OpacityManager opacityManager_;

    std::unique_ptr<SwapFile> swapFile_;
    std::unique_ptr<Autosave> autosave_;

    std::unique_ptr<RevisionMode> revisionMode_;
    std::unique_ptr<ConfirmationDialog> confirmationDialog_;
};

} // namespace phantom

#endif // PHANTOM_EDITOR_STATE_H
