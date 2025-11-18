#ifndef PHANTOM_REVISION_MODE_H
#define PHANTOM_REVISION_MODE_H

namespace phantom {

class RevisionMode {
public:
    RevisionMode();
    ~RevisionMode();

    // Check if revision mode is active
    bool isActive() const { return active_; }

    // Activate revision mode (after confirmation)
    void activate();

    // Deactivate revision mode
    void deactivate();

    // Check if fragmentation should be disabled
    bool shouldDisableFragmentation() const { return active_; }

    // Check if opacity should be disabled
    bool shouldDisableOpacity() const { return active_; }

    // Check if visual indicator should be shown
    bool shouldShowIndicator() const { return active_; }

private:
    bool active_;
};

} // namespace phantom

#endif // PHANTOM_REVISION_MODE_H
