#include "confirmation_dialog.h"
#include "utils/logger.h"
#include <cctype>
#include <cstring>

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

void ConfirmationDialog::processBackspace() {
    if (!active_ || currentInput_.empty()) {
        return;
    }

    currentInput_.pop_back();
    LOG_TRACE(LogCategory::UI, "Backspace in confirmation - current: '%s'", currentInput_.c_str());
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

std::string ConfirmationDialog::getPromptMessage() const {
    if (!active_) {
        return "";
    }

    std::string prompt = "Type 'REVELAR' to reveal text: ";
    prompt += currentInput_;

    // Add visual feedback showing remaining characters
    const size_t expectedLen = std::strlen(CONFIRMATION_TEXT);
    if (currentInput_.length() < expectedLen) {
        size_t remaining = expectedLen - currentInput_.length();
        prompt += " (" + std::to_string(remaining) + " more)";
    }

    return prompt;
}

void ConfirmationDialog::checkConfirmation() {
    const size_t inputLen = currentInput_.length();
    const size_t expectedLen = std::strlen(CONFIRMATION_TEXT);

    // Check if current input matches the confirmation text exactly
    if (inputLen == expectedLen && currentInput_ == CONFIRMATION_TEXT) {
        confirmed_ = true;
        active_ = false;
        LOG_INFO(LogCategory::UI, "Confirmation SUCCESSFUL - '%s' entered correctly", CONFIRMATION_TEXT);
        return;
    }

    // Check if input has diverged from expected text (wrong character)
    if (inputLen > 0 && inputLen <= expectedLen) {
        // Check if prefix matches expected text
        if (std::strncmp(currentInput_.c_str(), CONFIRMATION_TEXT, inputLen) != 0) {
            LOG_WARN(LogCategory::UI, "Wrong input - resetting (got '%s', expected prefix of '%s')",
                    currentInput_.c_str(), CONFIRMATION_TEXT);
            currentInput_.clear();
        }
    } else if (inputLen > expectedLen) {
        // Input too long - reset
        LOG_WARN(LogCategory::UI, "Input too long - resetting");
        currentInput_.clear();
    }
}

} // namespace phantom
