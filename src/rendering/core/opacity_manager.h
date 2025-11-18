#ifndef PHANTOM_OPACITY_MANAGER_H
#define PHANTOM_OPACITY_MANAGER_H

#include <cstddef>

namespace phantom {

class OpacityManager {
public:
    OpacityManager();
    ~OpacityManager();

    // Update the timer (call every frame with delta time in seconds)
    void update(float deltaTime);

    // Call this when user types or performs any editing action
    void onActivity();

    // Get opacity for a specific line
    // lineNumber: the line to get opacity for
    // currentLine: the line where cursor is currently
    // Returns: opacity value between 0.0 (fully transparent) and 1.0 (fully opaque)
    float getOpacityForLine(size_t lineNumber, size_t currentLine) const;

    // Get current opacity for previous lines (when typing)
    float getPreviousLinesOpacity() const { return previousLinesOpacity_; }

    // Check if we're currently in typing mode (active) or idle
    bool isIdle() const { return isIdle_; }

private:
    float timeSinceLastActivity_;  // Time in seconds since last activity
    bool isIdle_;                  // true if idle for > idleThreshold

    static constexpr float idleThreshold_ = 1.0f;        // 1 second
    static constexpr float previousLinesOpacity_ = 0.2f; // 20% opacity when typing
    static constexpr float fullOpacity_ = 1.0f;          // 100% opacity
};

} // namespace phantom

#endif // PHANTOM_OPACITY_MANAGER_H
