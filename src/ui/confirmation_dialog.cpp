#include "confirmation_dialog.h"
#include "utils/logger.h"
#include <cctype>

namespace phantom {

ConfirmationDialog::ConfirmationDialog()
    : active_(false)
    , confirmed_(false)
{
    LOG_DEBUG(LogCategory::UI, "ConfirmationDialog created");
}

ConfirmationDialog::~ConfirmationDialog() {
    LOG_TRACE(LogCategory::UI, "ConfirmationDialog destroyed");
}

void ConfirmationDialog::startConfirmation() {
    active_ = true;
    confirmed_ = false;
    currentInput_.clear();

    LOG_INFO(LogCategory::UI, "Confirmation dialog started - type '%s' to confirm", CONFIRMATION_TEXT);
}

void ConfirmationDialog::processInput(char ch) {
    if (!active_) {
        return;
    }

    // Convert to uppercase for comparison
    char upperCh = std::toupper(static_cast<unsigned char>(ch));

    // Add to current input
    currentInput_ += upperCh;

    LOG_TRACE(LogCategory::UI, "Confirmation input: '%s' (need: '%s')",
              currentInput_.c_str(), CONFIRMATION_TEXT);

    // Check if we've matched the confirmation text
    checkConfirmation();
}

void ConfirmationDialog::cancel() {
    if (!active_) {
        return;
    }

    active_ = false;
    confirmed_ = false;
    currentInput_.clear();

    LOG_INFO(LogCategory::UI, "Confirmation dialog cancelled");
}

void ConfirmationDialog::checkConfirmation() {
    // Check if current input matches the confirmation text
    if (currentInput_ == CONFIRMATION_TEXT) {
        confirmed_ = true;
        active_ = false;
        LOG_INFO(LogCategory::UI, "Confirmation SUCCESSFUL - '%s' entered correctly", CONFIRMATION_TEXT);
    }
    // Check if input has diverged from expected text (wrong character)
    else if (currentInput_.length() > 0) {
        std::string expected(CONFIRMATION_TEXT);
        if (currentInput_.length() <= expected.length()) {
            // Check if prefix matches
            if (expected.substr(0, currentInput_.length()) != currentInput_) {
                LOG_WARN(LogCategory::UI, "Wrong input - expected '%s', got '%s'",
                        expected.substr(0, currentInput_.length()).c_str(),
                        currentInput_.c_str());
                // Reset on wrong input
                currentInput_.clear();
            }
        } else {
            // Too long
            currentInput_.clear();
        }
    }
}

} // namespace phantom
