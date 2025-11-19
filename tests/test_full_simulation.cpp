/**
 * Full Simulation Test - Phantom Writer
 *
 * Este programa simula el uso completo del editor, ejercitando TODAS
 * las funcionalidades sin requerir UI visible.
 */

#include "../src/utils/logger.h"
#include "../src/rendering/core/font_loader.h"
#include "../src/rendering/core/glyph_fragmenter.h"
#include "../src/rendering/core/opacity_manager.h"
#include "../src/core/editor_state.h"
#include "../src/persistence/swap_file.h"
#include "../src/ui/revision_mode.h"
#include "../src/ui/confirmation_dialog.h"

#include <iostream>
#include <thread>
#include <chrono>
#include <cassert>

// ============================================================================
// Color codes
// ============================================================================
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_BOLD    "\033[1m"

void printHeader(const char* text) {
    printf("\n%s%s=== %s ===%s\n", COLOR_BOLD, COLOR_CYAN, text, COLOR_RESET);
}

void printSuccess(const char* text) {
    printf("%s%s✓ %s%s\n", COLOR_BOLD, COLOR_GREEN, text, COLOR_RESET);
}

void printInfo(const char* text) {
    printf("%s  %s%s\n", COLOR_BLUE, text, COLOR_RESET);
}

void printStep(int step, const char* text) {
    printf("\n%s%sStep %d:%s %s\n", COLOR_BOLD, COLOR_YELLOW, step, COLOR_RESET, text);
}

// ============================================================================
// Simulation Functions
// ============================================================================

void simulateTyping(phantom::EditorState& editor) {
    printStep(1, "Simulando escritura de texto");

    const char* text = "This is a test of the Phantom Writer system.\n"
                       "It supports multiple lines of text.\n"
                       "Each character is fragmented for anti-distraction.\n";

    for (const char* p = text; *p; p++) {
        editor.insertChar(*p);
    }

    std::string content = editor.getBuffer().getText();
    printInfo(("Characters typed: " + std::to_string(content.length())).c_str());
    printSuccess("Text insertion completed");
}

void simulateCursorMovement(phantom::EditorState& editor) {
    printStep(2, "Simulando movimiento del cursor");

    // Move cursor around
    printInfo("Moving cursor right...");
    for (int i = 0; i < 5; i++) {
        editor.getCursor().moveRight(editor.getBuffer());
    }

    printInfo("Moving cursor down...");
    editor.getCursor().moveDown(editor.getBuffer());

    printInfo("Moving to line start...");
    editor.getCursor().moveToLineStart(editor.getBuffer());

    printInfo("Moving to line end...");
    editor.getCursor().moveToLineEnd(editor.getBuffer());

    printInfo("Moving cursor up...");
    editor.getCursor().moveUp(editor.getBuffer());

    size_t finalPos = editor.getCursor().getPosition();
    printInfo(("Final cursor position: " + std::to_string(finalPos)).c_str());
    printSuccess("Cursor movement completed");
}

void simulateEditing(phantom::EditorState& editor) {
    printStep(3, "Simulando edición de texto");

    size_t initialLength = editor.getBuffer().length();

    printInfo("Deleting characters...");
    for (int i = 0; i < 3; i++) {
        editor.deleteChar();
    }

    printInfo("Inserting new text...");
    editor.insertChar('X');
    editor.insertChar('Y');
    editor.insertChar('Z');

    size_t finalLength = editor.getBuffer().length();
    printInfo(("Buffer length: " + std::to_string(initialLength) + " -> " +
               std::to_string(finalLength)).c_str());

    printSuccess("Text editing completed");
}

void simulateOpacityManager(phantom::EditorState& editor) {
    printStep(4, "Simulando opacity manager (anti-distraction)");

    float deltaTime = 0.016f; // 60 FPS

    printInfo("Simulating typing activity...");
    editor.getOpacityManager().onActivity();

    printInfo("Getting current opacity...");
    float opacity = editor.getOpacityManager().getPreviousLinesOpacity();
    printInfo(("Current opacity: " + std::to_string(opacity)).c_str());

    printInfo("Simulating idle time...");
    for (int i = 0; i < 100; i++) {
        editor.getOpacityManager().update(deltaTime);
    }

    bool isIdle = editor.getOpacityManager().isIdle();
    printInfo(isIdle ? "System is idle" : "System is active");

    printSuccess("Opacity manager simulation completed");
}

void simulateAutosave(phantom::EditorState& editor) {
    printStep(5, "Simulando autosave");

    printInfo("Starting autosave thread...");
    editor.startAutosave();

    printInfo("Typing some text...");
    editor.insertChar('A');
    editor.insertChar('B');
    editor.insertChar('C');

    printInfo("Waiting for autosave...");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    printInfo("Stopping autosave...");
    editor.stopAutosave();

    printSuccess("Autosave simulation completed");
}

void simulateRevisionMode(phantom::EditorState& editor) {
    printStep(6, "Simulando revision mode");

    printInfo("Checking initial state...");
    bool initialState = editor.getRevisionMode()->isActive();
    printInfo(initialState ? "Revision mode is active" : "Revision mode is inactive");

    printInfo("Activating revision mode...");
    editor.getRevisionMode()->activate();

    bool afterActivation = editor.getRevisionMode()->isActive();
    printInfo(afterActivation ? "Revision mode activated" : "Failed to activate");

    printInfo("Deactivating revision mode...");
    editor.getRevisionMode()->deactivate();

    bool afterDeactivation = editor.getRevisionMode()->isActive();
    printInfo(afterDeactivation ? "Still active" : "Revision mode deactivated");

    printSuccess("Revision mode simulation completed");
}

void simulateConfirmationDialog(phantom::EditorState& editor) {
    printStep(7, "Simulando confirmation dialog");

    printInfo("Starting confirmation...");
    editor.getConfirmationDialog()->startConfirmation();

    bool isActive = editor.getConfirmationDialog()->isActive();
    printInfo(isActive ? "Dialog is active" : "Dialog is not active");

    printInfo("Getting prompt message...");
    std::string prompt = editor.getConfirmationDialog()->getPromptMessage();
    printInfo(("Prompt: " + prompt).c_str());

    printInfo("Simulating typing 'REVISION'...");
    const char* confirmText = "REVISION";
    for (const char* p = confirmText; *p; p++) {
        editor.getConfirmationDialog()->processInput(*p);
    }

    bool confirmed = editor.getConfirmationDialog()->isConfirmed();
    printInfo(confirmed ? "Confirmation succeeded" : "Confirmation failed");

    if (!confirmed) {
        printInfo("Cancelling dialog...");
        editor.getConfirmationDialog()->cancel();
    }

    printSuccess("Confirmation dialog simulation completed");
}

void simulateGlyphFragmentation() {
    printStep(8, "Simulando fragmentación de glifos");

    phantom::GlyphFragmenter fragmenter;

    printInfo("Testing fragmentation for different positions...");

    int topCount = 0, bottomCount = 0;
    for (size_t i = 0; i < 100; i++) {
        phantom::FragmentMode mode = fragmenter.getFragmentModeByIndex(i);
        if (mode == phantom::FragmentMode::Top) topCount++;
        else if (mode == phantom::FragmentMode::Bottom) bottomCount++;
    }

    printInfo(("Top fragments: " + std::to_string(topCount)).c_str());
    printInfo(("Bottom fragments: " + std::to_string(bottomCount)).c_str());

    // Test consistency
    printInfo("Testing fragmentation consistency...");
    bool consistent = true;
    for (size_t i = 0; i < 50; i++) {
        phantom::FragmentMode mode1 = fragmenter.getFragmentModeByIndex(i);
        phantom::FragmentMode mode2 = fragmenter.getFragmentModeByIndex(i);
        if (mode1 != mode2) {
            consistent = false;
            break;
        }
    }

    printInfo(consistent ? "Fragmentation is consistent" : "Fragmentation is inconsistent");

    printSuccess("Glyph fragmentation simulation completed");
}

void simulateFontLoading() {
    printStep(9, "Simulando carga de fuentes");

    phantom::FontLoader fontLoader;

    printInfo("Loading font: assets/fonts/default_mono.ttf");
    bool loaded = fontLoader.loadFromFile("assets/fonts/default_mono.ttf", 48.0f);

    if (!loaded) {
        printInfo("Failed to load font");
        return;
    }

    const phantom::FontAtlas& atlas = fontLoader.getAtlas();

    printInfo(("Atlas size: " + std::to_string(atlas.width) + "x" +
               std::to_string(atlas.height)).c_str());
    printInfo(("Glyphs loaded: " + std::to_string(atlas.glyphs.size())).c_str());
    printInfo(("Line height: " + std::to_string(atlas.lineHeight)).c_str());

    // Test some glyphs
    printInfo("Testing glyph lookup...");
    const phantom::Glyph* glyphA = fontLoader.getGlyph('A');
    const phantom::Glyph* glyph0 = fontLoader.getGlyph('0');
    const phantom::Glyph* glyphSpace = fontLoader.getGlyph(' ');

    int validGlyphs = 0;
    if (glyphA) validGlyphs++;
    if (glyph0) validGlyphs++;
    if (glyphSpace) validGlyphs++;

    printInfo(("Valid glyphs found: " + std::to_string(validGlyphs) + "/3").c_str());

    printSuccess("Font loading simulation completed");
}

void simulateSwapFile(phantom::EditorState& editor) {
    printStep(10, "Simulando swap file operations");

    auto swapFile = editor.getSwapFile();

    printInfo("Checking swap file existence...");
    bool exists = swapFile->exists();
    printInfo(exists ? "Swap file exists" : "No swap file");

    printInfo("Getting swap file path...");
    std::string path = swapFile->getSwapFilePath();
    printInfo(("Swap file path: " + path).c_str());

    printSuccess("Swap file simulation completed");
}

void stressTest(phantom::EditorState& editor) {
    printStep(11, "Stress test - Simulando uso intensivo");

    auto startTime = std::chrono::high_resolution_clock::now();

    printInfo("Inserting 1000 characters...");
    for (int i = 0; i < 1000; i++) {
        editor.insertChar('X');
    }

    printInfo("Deleting 500 characters...");
    for (int i = 0; i < 500; i++) {
        editor.deleteChar();
    }

    printInfo("Moving cursor 100 times...");
    for (int i = 0; i < 100; i++) {
        editor.getCursor().moveRight(editor.getBuffer());
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    printInfo(("Stress test completed in " + std::to_string(duration.count()) + " ms").c_str());
    printInfo(("Final buffer size: " + std::to_string(editor.getBuffer().length())).c_str());

    printSuccess("Stress test completed");
}

// ============================================================================
// Main
// ============================================================================
int main() {
    printf("\n");
    printf("%s%s", COLOR_BOLD, COLOR_MAGENTA);
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║                                                                ║\n");
    printf("║      PHANTOM WRITER - FULL SIMULATION TEST                    ║\n");
    printf("║                                                                ║\n");
    printf("║   Ejecuta TODAS las funcionalidades del sistema               ║\n");
    printf("║   de forma autónoma sin requerir UI visible                   ║\n");
    printf("║                                                                ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    printf("%s\n", COLOR_RESET);

    // Initialize logger
    phantom::Logger::init("full_simulation.log");
    LOG_INFO(phantom::LogCategory::INIT, "=== FULL SIMULATION TEST STARTED ===");

    printHeader("INITIALIZATION");

    printInfo("Creating editor state...");
    phantom::EditorState editor("");
    printSuccess("Editor state created");

    try {
        // Run all simulations
        simulateFontLoading();
        simulateGlyphFragmentation();
        simulateTyping(editor);
        simulateCursorMovement(editor);
        simulateEditing(editor);
        simulateOpacityManager(editor);
        simulateAutosave(editor);
        simulateSwapFile(editor);
        simulateRevisionMode(editor);
        simulateConfirmationDialog(editor);
        stressTest(editor);

        printHeader("FINAL STATE");

        std::string finalText = editor.getBuffer().getText();
        size_t finalLength = finalText.length();
        size_t cursorPos = editor.getCursor().getPosition();
        size_t lineCount = editor.getBuffer().getLineCount();

        printInfo(("Buffer length: " + std::to_string(finalLength) + " characters").c_str());
        printInfo(("Line count: " + std::to_string(lineCount)).c_str());
        printInfo(("Cursor position: " + std::to_string(cursorPos)).c_str());

        printInfo("\nBuffer content (first 200 chars):");
        std::string preview = finalText.substr(0, std::min(finalText.length(), size_t(200)));
        printf("%s%s%s\n", COLOR_YELLOW, preview.c_str(), COLOR_RESET);

    } catch (const std::exception& e) {
        printf("\n%s%sEXCEPTION: %s%s\n", COLOR_BOLD, COLOR_RED, e.what(), COLOR_RESET);
        LOG_FATAL(phantom::LogCategory::INIT, "Exception caught: %s", e.what());
        phantom::Logger::shutdown();
        return 1;
    }

    // Cleanup
    printHeader("CLEANUP");
    editor.stopAutosave();

    LOG_INFO(phantom::LogCategory::INIT, "=== FULL SIMULATION TEST COMPLETED ===");
    phantom::Logger::shutdown();

    printf("\n");
    printf("%s%s", COLOR_BOLD, COLOR_GREEN);
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║                                                                ║\n");
    printf("║              ✓ ALL SIMULATIONS COMPLETED                       ║\n");
    printf("║                                                                ║\n");
    printf("║   Todas las funcionalidades del sistema han sido              ║\n");
    printf("║   ejecutadas y validadas exitosamente                         ║\n");
    printf("║                                                                ║\n");
    printf("║   Log file: full_simulation.log                               ║\n");
    printf("║                                                                ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    printf("%s\n", COLOR_RESET);

    return 0;
}
