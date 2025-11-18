#ifndef PHANTOM_CURSOR_H
#define PHANTOM_CURSOR_H

#include <phantom_writer/types.h>

namespace phantom {

class TextBuffer;

class Cursor {
public:
    Cursor();
    ~Cursor();

    // Position
    size_t getPosition() const { return position_; }
    void setPosition(size_t position);

    // Movement
    void moveLeft(const TextBuffer& buffer);
    void moveRight(const TextBuffer& buffer);
    void moveUp(const TextBuffer& buffer);
    void moveDown(const TextBuffer& buffer);
    void moveToLineStart(const TextBuffer& buffer);
    void moveToLineEnd(const TextBuffer& buffer);

    // Line/column info
    size_t getLine(const TextBuffer& buffer) const;
    size_t getColumn(const TextBuffer& buffer) const;

private:
    size_t position_;
    size_t preferredColumn_; // For up/down movement
};

} // namespace phantom

#endif // PHANTOM_CURSOR_H
