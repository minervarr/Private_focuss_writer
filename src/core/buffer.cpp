#include "buffer.h"
#include "utils/logger.h"
#include <algorithm>

namespace phantom {

TextBuffer::TextBuffer() : gapStart_(0), gapEnd_(INITIAL_GAP_SIZE) {
    buffer_.resize(INITIAL_GAP_SIZE);
    LOG_TRACE(LogCategory::BUFFER, "TextBuffer created with gap size %zu", INITIAL_GAP_SIZE);
}

TextBuffer::~TextBuffer() {
    LOG_TRACE(LogCategory::BUFFER, "TextBuffer destroyed");
}

void TextBuffer::moveGap(size_t position) {
    if (position == gapStart_) {
        return;
    }

    if (position < gapStart_) {
        // Move gap left
        size_t count = gapStart_ - position;
        size_t gapSize = gapEnd_ - gapStart_;

        // Move text from [position, gapStart) to [gapEnd - count, gapEnd)
        for (size_t i = 0; i < count; i++) {
            buffer_[gapEnd_ - count + i] = buffer_[position + i];
        }

        gapEnd_ -= count;
        gapStart_ -= count;
    } else {
        // Move gap right
        size_t count = position - gapStart_;
        size_t gapSize = gapEnd_ - gapStart_;

        // Move text from [gapEnd, gapEnd + count) to [gapStart, gapStart + count)
        for (size_t i = 0; i < count; i++) {
            buffer_[gapStart_ + i] = buffer_[gapEnd_ + i];
        }

        gapStart_ += count;
        gapEnd_ += count;
    }
}

void TextBuffer::expandGap(size_t minSize) {
    size_t currentGapSize = gapEnd_ - gapStart_;
    if (currentGapSize >= minSize) {
        return;
    }

    size_t newGapSize = std::max(minSize, currentGapSize * 2);
    size_t additionalSize = newGapSize - currentGapSize;

    std::string newBuffer;
    newBuffer.resize(buffer_.size() + additionalSize);

    // Copy text before gap
    for (size_t i = 0; i < gapStart_; i++) {
        newBuffer[i] = buffer_[i];
    }

    // Copy text after gap
    size_t afterGapCount = buffer_.size() - gapEnd_;
    for (size_t i = 0; i < afterGapCount; i++) {
        newBuffer[gapStart_ + newGapSize + i] = buffer_[gapEnd_ + i];
    }

    buffer_ = std::move(newBuffer);
    gapEnd_ = gapStart_ + newGapSize;

    LOG_DEBUG(LogCategory::BUFFER, "Gap expanded to %zu bytes", newGapSize);
}

void TextBuffer::insert(size_t position, char ch) {
    moveGap(position);
    expandGap(1);

    buffer_[gapStart_] = ch;
    gapStart_++;

    LOG_TRACE(LogCategory::BUFFER, "Insert '%c' at pos %zu", ch, position);
}

void TextBuffer::insert(size_t position, const std::string& text) {
    if (text.empty()) {
        return;
    }

    moveGap(position);
    expandGap(text.length());

    for (size_t i = 0; i < text.length(); i++) {
        buffer_[gapStart_ + i] = text[i];
    }

    gapStart_ += text.length();

    LOG_TRACE(LogCategory::BUFFER, "Insert \"%s\" (%zu chars) at pos %zu",
        text.c_str(), text.length(), position);
}

void TextBuffer::erase(size_t position, size_t length) {
    if (length == 0 || position >= this->length()) {
        return;
    }

    // Clamp length to available text
    length = std::min(length, this->length() - position);

    moveGap(position);
    gapEnd_ += length;

    LOG_TRACE(LogCategory::BUFFER, "Erase %zu chars at pos %zu", length, position);
}

void TextBuffer::clear() {
    gapStart_ = 0;
    gapEnd_ = buffer_.size();
    LOG_DEBUG(LogCategory::BUFFER, "Buffer cleared");
}

size_t TextBuffer::length() const {
    return buffer_.size() - (gapEnd_ - gapStart_);
}

std::string TextBuffer::getText() const {
    std::string result;
    result.reserve(length());

    // Text before gap
    for (size_t i = 0; i < gapStart_; i++) {
        result += buffer_[i];
    }

    // Text after gap
    for (size_t i = gapEnd_; i < buffer_.size(); i++) {
        result += buffer_[i];
    }

    return result;
}

std::string TextBuffer::getText(size_t start, size_t length) const {
    if (start >= this->length()) {
        return "";
    }

    length = std::min(length, this->length() - start);

    std::string result;
    result.reserve(length);

    for (size_t i = 0; i < length; i++) {
        result += getChar(start + i);
    }

    return result;
}

char TextBuffer::getChar(size_t position) const {
    if (position >= length()) {
        return '\0';
    }

    if (position < gapStart_) {
        return buffer_[position];
    } else {
        return buffer_[position + (gapEnd_ - gapStart_)];
    }
}

std::string TextBuffer::getLine(size_t lineNumber) const {
    size_t start = lineStartPosition(lineNumber);
    size_t end = lineEndPosition(lineNumber);

    if (start == SIZE_MAX || end == SIZE_MAX) {
        return "";
    }

    return getText(start, end - start);
}

size_t TextBuffer::getLineCount() const {
    size_t count = 1; // At least one line
    size_t len = length();

    for (size_t i = 0; i < len; i++) {
        if (getChar(i) == '\n') {
            count++;
        }
    }

    return count;
}

size_t TextBuffer::lineStartPosition(size_t lineNumber) const {
    if (lineNumber == 0) {
        return 0;
    }

    size_t currentLine = 0;
    size_t len = length();

    for (size_t i = 0; i < len; i++) {
        if (getChar(i) == '\n') {
            currentLine++;
            if (currentLine == lineNumber) {
                return i + 1;
            }
        }
    }

    return SIZE_MAX; // Line not found
}

size_t TextBuffer::lineEndPosition(size_t lineNumber) const {
    size_t start = lineStartPosition(lineNumber);
    if (start == SIZE_MAX) {
        return SIZE_MAX;
    }

    size_t len = length();
    for (size_t i = start; i < len; i++) {
        if (getChar(i) == '\n') {
            return i;
        }
    }

    return len; // End of buffer
}

size_t TextBuffer::positionToLine(size_t position) const {
    size_t line = 0;
    for (size_t i = 0; i < position && i < length(); i++) {
        if (getChar(i) == '\n') {
            line++;
        }
    }
    return line;
}

size_t TextBuffer::positionToColumn(size_t position) const {
    size_t lineStart = lineStartPosition(positionToLine(position));
    return position - lineStart;
}

} // namespace phantom
