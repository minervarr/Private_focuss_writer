#include "cursor.h"
#include "buffer.h"
#include "utils/logger.h"
#include <algorithm>

namespace phantom {

Cursor::Cursor() : position_(0), preferredColumn_(0) {
    LOG_TRACE(LogCategory::BUFFER, "Cursor created");
}

Cursor::~Cursor() {
    LOG_TRACE(LogCategory::BUFFER, "Cursor destroyed");
}

void Cursor::setPosition(size_t position) {
    position_ = position;
    LOG_TRACE(LogCategory::BUFFER, "Cursor position set to %zu", position_);
}

void Cursor::moveLeft(const TextBuffer& buffer) {
    if (position_ > 0) {
        position_--;
        preferredColumn_ = buffer.positionToColumn(position_);
        LOG_TRACE(LogCategory::BUFFER, "Cursor moved left to %zu", position_);
    }
}

void Cursor::moveRight(const TextBuffer& buffer) {
    if (position_ < buffer.length()) {
        position_++;
        preferredColumn_ = buffer.positionToColumn(position_);
        LOG_TRACE(LogCategory::BUFFER, "Cursor moved right to %zu", position_);
    }
}

void Cursor::moveUp(const TextBuffer& buffer) {
    size_t currentLine = buffer.positionToLine(position_);

    if (currentLine == 0) {
        // Already at first line, move to start
        position_ = 0;
        return;
    }

    size_t prevLineStart = buffer.lineStartPosition(currentLine - 1);
    size_t prevLineEnd = buffer.lineEndPosition(currentLine - 1);
    size_t prevLineLength = prevLineEnd - prevLineStart;

    // Try to maintain column position
    size_t newColumn = std::min(preferredColumn_, prevLineLength);
    position_ = prevLineStart + newColumn;

    LOG_TRACE(LogCategory::BUFFER, "Cursor moved up to line %zu, pos %zu",
        currentLine - 1, position_);
}

void Cursor::moveDown(const TextBuffer& buffer) {
    size_t currentLine = buffer.positionToLine(position_);
    size_t totalLines = buffer.getLineCount();

    if (currentLine >= totalLines - 1) {
        // Already at last line, move to end
        position_ = buffer.length();
        return;
    }

    size_t nextLineStart = buffer.lineStartPosition(currentLine + 1);
    size_t nextLineEnd = buffer.lineEndPosition(currentLine + 1);
    size_t nextLineLength = nextLineEnd - nextLineStart;

    // Try to maintain column position
    size_t newColumn = std::min(preferredColumn_, nextLineLength);
    position_ = nextLineStart + newColumn;

    LOG_TRACE(LogCategory::BUFFER, "Cursor moved down to line %zu, pos %zu",
        currentLine + 1, position_);
}

void Cursor::moveToLineStart(const TextBuffer& buffer) {
    size_t currentLine = buffer.positionToLine(position_);
    position_ = buffer.lineStartPosition(currentLine);
    preferredColumn_ = 0;

    LOG_TRACE(LogCategory::BUFFER, "Cursor moved to line start, pos %zu", position_);
}

void Cursor::moveToLineEnd(const TextBuffer& buffer) {
    size_t currentLine = buffer.positionToLine(position_);
    position_ = buffer.lineEndPosition(currentLine);
    preferredColumn_ = buffer.positionToColumn(position_);

    LOG_TRACE(LogCategory::BUFFER, "Cursor moved to line end, pos %zu", position_);
}

size_t Cursor::getLine(const TextBuffer& buffer) const {
    return buffer.positionToLine(position_);
}

size_t Cursor::getColumn(const TextBuffer& buffer) const {
    return buffer.positionToColumn(position_);
}

} // namespace phantom
