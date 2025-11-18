#ifndef PHANTOM_EDITOR_STATE_H
#define PHANTOM_EDITOR_STATE_H

#include "buffer.h"
#include "cursor.h"

namespace phantom {

// Simple editor state that holds buffer and cursor
class EditorState {
public:
    EditorState() = default;
    ~EditorState() = default;

    TextBuffer& getBuffer() { return buffer_; }
    const TextBuffer& getBuffer() const { return buffer_; }

    Cursor& getCursor() { return cursor_; }
    const Cursor& getCursor() const { return cursor_; }

    // Convenience methods
    void insertChar(char ch) {
        buffer_.insert(cursor_.getPosition(), ch);
        cursor_.setPosition(cursor_.getPosition() + 1);
    }

    void deleteChar() {
        if (cursor_.getPosition() > 0) {
            size_t pos = cursor_.getPosition() - 1;
            buffer_.erase(pos, 1);
            cursor_.setPosition(pos);
        }
    }

private:
    TextBuffer buffer_;
    Cursor cursor_;
};

} // namespace phantom

#endif // PHANTOM_EDITOR_STATE_H
