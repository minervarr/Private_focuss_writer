#ifndef PHANTOM_EDITOR_STATE_H
#define PHANTOM_EDITOR_STATE_H

#include "buffer.h"
#include "cursor.h"
#include "rendering/core/opacity_manager.h"

namespace phantom {

// Simple editor state that holds buffer, cursor, and opacity manager
class EditorState {
public:
    EditorState() = default;
    ~EditorState() = default;

    TextBuffer& getBuffer() { return buffer_; }
    const TextBuffer& getBuffer() const { return buffer_; }

    Cursor& getCursor() { return cursor_; }
    const Cursor& getCursor() const { return cursor_; }

    OpacityManager& getOpacityManager() { return opacityManager_; }
    const OpacityManager& getOpacityManager() const { return opacityManager_; }

    // Convenience methods
    void insertChar(char ch) {
        buffer_.insert(cursor_.getPosition(), ch);
        cursor_.setPosition(cursor_.getPosition() + 1);
        opacityManager_.onActivity(); // Notify activity
    }

    void deleteChar() {
        if (cursor_.getPosition() > 0) {
            size_t pos = cursor_.getPosition() - 1;
            buffer_.erase(pos, 1);
            cursor_.setPosition(pos);
            opacityManager_.onActivity(); // Notify activity
        }
    }

    void moveCursor(size_t newPosition) {
        cursor_.setPosition(newPosition);
        opacityManager_.onActivity(); // Notify activity
    }

private:
    TextBuffer buffer_;
    Cursor cursor_;
    OpacityManager opacityManager_;
};

} // namespace phantom

#endif // PHANTOM_EDITOR_STATE_H
