#ifndef PHANTOM_AUTOSAVE_H
#define PHANTOM_AUTOSAVE_H

#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <memory>

namespace phantom {

class SwapFile;
class TextBuffer;
class Cursor;

class Autosave {
public:
    Autosave(SwapFile* swapFile, const TextBuffer& buffer, const Cursor& cursor);
    ~Autosave();

    // Start autosave thread
    void start();

    // Stop autosave thread
    void stop();

    // Trigger immediate save (called by Ctrl+S)
    void saveNow();

    // Mark buffer as modified (restart timer)
    void markDirty();

    // Check if autosave is running
    bool isRunning() const { return running_.load(); }

private:
    void autosaveLoop();

    SwapFile* swapFile_;
    const TextBuffer& buffer_;
    const Cursor& cursor_;

    std::thread autosaveThread_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::atomic<bool> running_;
    std::atomic<bool> shouldExit_;
    std::atomic<bool> isDirty_;

    static constexpr float AUTOSAVE_INTERVAL = 3.0f; // 3 seconds
};

} // namespace phantom

#endif // PHANTOM_AUTOSAVE_H
