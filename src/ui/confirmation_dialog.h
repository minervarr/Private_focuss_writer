#ifndef PHANTOM_CONFIRMATION_DIALOG_H
#define PHANTOM_CONFIRMATION_DIALOG_H

#include <string>

namespace phantom {

class ConfirmationDialog {
public:
    ConfirmationDialog();
    ~ConfirmationDialog();

    // Start confirmation process
    void startConfirmation();

    // Process a character input for confirmation
    void processInput(char ch);

    // Cancel confirmation
    void cancel();

    // Check if dialog is active
    bool isActive() const { return active_; }

    // Check if confirmation is complete
    bool isConfirmed() const { return confirmed_; }

    // Get current input
    const std::string& getCurrentInput() const { return currentInput_; }

    // Get expected confirmation text
    static constexpr const char* CONFIRMATION_TEXT = "REVELAR";

private:
    void checkConfirmation();

    bool active_;
    bool confirmed_;
    std::string currentInput_;
};

} // namespace phantom

#endif // PHANTOM_CONFIRMATION_DIALOG_H
