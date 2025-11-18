#include "swap_file.h"
#include "core/buffer.h"
#include "core/cursor.h"
#include "utils/logger.h"

#include <fstream>
#include <sstream>
#include <ctime>
#include <sys/stat.h>

namespace phantom {

SwapFile::SwapFile(const std::string& originalFilePath)
    : originalFilePath_(originalFilePath)
{
    // Create swap file path: .filename.swp
    if (originalFilePath.empty()) {
        swapFilePath_ = ".phantom_untitled.swp";
    } else {
        size_t lastSlash = originalFilePath.find_last_of("/\\");
        std::string dir = (lastSlash != std::string::npos) ? originalFilePath.substr(0, lastSlash + 1) : "";
        std::string filename = (lastSlash != std::string::npos) ? originalFilePath.substr(lastSlash + 1) : originalFilePath;
        swapFilePath_ = dir + "." + filename + ".swp";
    }

    LOG_DEBUG(LogCategory::PERSISTENCE, "SwapFile created: %s", swapFilePath_.c_str());
}

SwapFile::~SwapFile() {
    LOG_TRACE(LogCategory::PERSISTENCE, "SwapFile destroyed");
}

bool SwapFile::write(const TextBuffer& buffer, const Cursor& cursor) {
    LOG_TRACE(LogCategory::PERSISTENCE, "Writing swap file: %s", swapFilePath_.c_str());

    std::ofstream file(swapFilePath_, std::ios::binary | std::ios::trunc);
    if (!file.is_open()) {
        LOG_ERROR(LogCategory::PERSISTENCE, "Failed to open swap file for writing: %s", swapFilePath_.c_str());
        return false;
    }

    // Get current timestamp
    time_t now = time(nullptr);

    // Get cursor position (line and column)
    size_t cursorPos = cursor.getPosition();
    size_t cursorLine = buffer.positionToLine(cursorPos);
    size_t cursorCol = cursor.getPreferredColumn();

    // Get buffer content
    std::string content = buffer.getText();

    // Write header
    file << SWAP_HEADER << "\n";
    file << "timestamp: " << now << "\n";
    file << "cursor_line: " << cursorLine << "\n";
    file << "cursor_column: " << cursorCol << "\n";
    file << "cursor_position: " << cursorPos << "\n";
    file << "buffer_length: " << content.length() << "\n";
    file << "---BEGIN_CONTENT---\n";
    file << content;
    file << "\n---END_CONTENT---\n";

    file.close();

    if (file.fail()) {
        LOG_ERROR(LogCategory::PERSISTENCE, "Error writing swap file: %s", swapFilePath_.c_str());
        return false;
    }

    LOG_INFO(LogCategory::PERSISTENCE, "Swap file written: %zu bytes", content.length());
    return true;
}

bool SwapFile::exists() const {
    std::ifstream file(swapFilePath_);
    return file.good();
}

bool SwapFile::read(TextBuffer& buffer, Cursor& cursor) {
    LOG_INFO(LogCategory::PERSISTENCE, "Reading swap file: %s", swapFilePath_.c_str());

    std::ifstream file(swapFilePath_, std::ios::binary);
    if (!file.is_open()) {
        LOG_ERROR(LogCategory::PERSISTENCE, "Failed to open swap file for reading: %s", swapFilePath_.c_str());
        return false;
    }

    std::string line;

    // Read and validate header
    std::getline(file, line);
    if (line != SWAP_HEADER) {
        LOG_ERROR(LogCategory::PERSISTENCE, "Invalid swap file header: %s", line.c_str());
        return false;
    }

    // Parse metadata
    long timestamp = 0;
    size_t cursorLine = 0;
    size_t cursorCol = 0;
    size_t cursorPos = 0;
    size_t bufferLength = 0;

    while (std::getline(file, line)) {
        if (line == "---BEGIN_CONTENT---") {
            break;
        }

        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string key = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 2); // Skip ": "

            if (key == "timestamp") {
                timestamp = std::stol(value);
            } else if (key == "cursor_line") {
                cursorLine = std::stoull(value);
            } else if (key == "cursor_column") {
                cursorCol = std::stoull(value);
            } else if (key == "cursor_position") {
                cursorPos = std::stoull(value);
            } else if (key == "buffer_length") {
                bufferLength = std::stoull(value);
            }
        }
    }

    // Read content
    std::stringstream contentStream;
    std::string contentLine;
    while (std::getline(file, contentLine)) {
        if (contentLine == "---END_CONTENT---") {
            break;
        }
        if (contentStream.tellp() > 0) {
            contentStream << "\n";
        }
        contentStream << contentLine;
    }

    std::string content = contentStream.str();
    file.close();

    LOG_INFO(LogCategory::PERSISTENCE, "Swap file read: timestamp=%ld, length=%zu", timestamp, content.length());

    // Restore buffer
    buffer.clear();
    if (!content.empty()) {
        buffer.insert(0, content);
    }

    // Restore cursor
    cursor.setPosition(cursorPos);
    cursor.setPreferredColumn(cursorCol);

    return true;
}

bool SwapFile::remove() {
    LOG_INFO(LogCategory::PERSISTENCE, "Removing swap file: %s", swapFilePath_.c_str());

    if (std::remove(swapFilePath_.c_str()) == 0) {
        LOG_DEBUG(LogCategory::PERSISTENCE, "Swap file removed successfully");
        return true;
    } else {
        LOG_WARN(LogCategory::PERSISTENCE, "Failed to remove swap file: %s", swapFilePath_.c_str());
        return false;
    }
}

bool SwapFile::isNewerThanOriginal() const {
    if (!exists()) {
        return false;
    }

    struct stat swapStat, origStat;

    if (stat(swapFilePath_.c_str(), &swapStat) != 0) {
        return false;
    }

    if (originalFilePath_.empty() || stat(originalFilePath_.c_str(), &origStat) != 0) {
        // Original file doesn't exist, swap is "newer"
        return true;
    }

    return swapStat.st_mtime > origStat.st_mtime;
}

} // namespace phantom
