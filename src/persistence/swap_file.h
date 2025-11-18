#ifndef PHANTOM_SWAP_FILE_H
#define PHANTOM_SWAP_FILE_H

#include <string>
#include <cstddef>

namespace phantom {

class TextBuffer;
class Cursor;

class SwapFile {
public:
    SwapFile(const std::string& originalFilePath);
    ~SwapFile();

    // Write current state to swap file
    bool write(const TextBuffer& buffer, const Cursor& cursor);

    // Check if swap file exists
    bool exists() const;

    // Read swap file and restore state
    bool read(TextBuffer& buffer, Cursor& cursor);

    // Delete swap file (after successful save)
    bool remove();

    // Get swap file path
    std::string getSwapFilePath() const { return swapFilePath_; }

    // Check if swap file is newer than original file
    bool isNewerThanOriginal() const;

private:
    std::string originalFilePath_;
    std::string swapFilePath_;

    static constexpr const char* SWAP_HEADER = "PHANTOM_SWAP_V1";
};

} // namespace phantom

#endif // PHANTOM_SWAP_FILE_H
