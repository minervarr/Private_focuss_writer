#ifndef PHANTOM_BUFFER_H
#define PHANTOM_BUFFER_H

#include <phantom_writer/types.h>
#include <string>
#include <memory>

namespace phantom {

// Simple gap buffer implementation for text editing
// Optimized for cursor-based insertion/deletion
class TextBuffer {
public:
    TextBuffer();
    ~TextBuffer();

    // Operaciones básicas
    void insert(size_t position, char ch);
    void insert(size_t position, const std::string& text);
    void erase(size_t position, size_t length = 1);
    void clear();

    // Queries
    size_t length() const;
    std::string getText() const;
    std::string getText(size_t start, size_t length) const;
    std::string getLine(size_t lineNumber) const;
    size_t getLineCount() const;
    char getChar(size_t position) const;

    // Cursor utilities
    size_t lineStartPosition(size_t lineNumber) const;
    size_t lineEndPosition(size_t lineNumber) const;
    size_t positionToLine(size_t position) const;
    size_t positionToColumn(size_t position) const;

private:
    void moveGap(size_t position);
    void expandGap(size_t minSize);

    std::string buffer_;
    size_t gapStart_;
    size_t gapEnd_;

    static constexpr size_t INITIAL_GAP_SIZE = 128;
    static constexpr size_t MIN_GAP_SIZE = 64;
};

} // namespace phantom

#endif // PHANTOM_BUFFER_H
