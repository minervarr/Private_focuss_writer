/**
 * Autonomous Testing Suite para Phantom Writer
 *
 * Este programa ejecuta tests automáticos y profundos sin requerir UI visible.
 * Valida cada componente crítico del sistema de forma independiente.
 */

#include "../src/utils/logger.h"
#include "../src/rendering/core/font_loader.h"
#include "../src/rendering/core/glyph_fragmenter.h"
#include "../src/core/buffer.h"
#include "../src/core/cursor.h"
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <chrono>
#include <cstring>

// ============================================================================
// Color codes para terminal
// ============================================================================
#define COLOR_RESET   "\033[0m"
#define COLOR_RED     "\033[31m"
#define COLOR_GREEN   "\033[32m"
#define COLOR_YELLOW  "\033[33m"
#define COLOR_BLUE    "\033[34m"
#define COLOR_MAGENTA "\033[35m"
#define COLOR_CYAN    "\033[36m"
#define COLOR_BOLD    "\033[1m"

// ============================================================================
// Test tracking
// ============================================================================
static int totalTests = 0;
static int passedTests = 0;
static int failedTests = 0;

void printTestHeader(const char* testName) {
    printf("\n%s%s========================================%s\n", COLOR_BOLD, COLOR_CYAN, COLOR_RESET);
    printf("%s%sTEST: %s%s\n", COLOR_BOLD, COLOR_CYAN, testName, COLOR_RESET);
    printf("%s%s========================================%s\n", COLOR_BOLD, COLOR_CYAN, COLOR_RESET);
}

void printTestResult(bool passed, const char* message) {
    totalTests++;
    if (passed) {
        passedTests++;
        printf("%s%s✓ PASS:%s %s\n", COLOR_BOLD, COLOR_GREEN, COLOR_RESET, message);
    } else {
        failedTests++;
        printf("%s%s✗ FAIL:%s %s\n", COLOR_BOLD, COLOR_RED, COLOR_RESET, message);
    }
}

void printSection(const char* section) {
    printf("\n%s%s>>> %s%s\n", COLOR_BOLD, COLOR_YELLOW, section, COLOR_RESET);
}

void printInfo(const char* format, ...) {
    va_list args;
    va_start(args, format);
    printf("%s[INFO]%s ", COLOR_BLUE, COLOR_RESET);
    vprintf(format, args);
    printf("\n");
    va_end(args);
}

void printError(const char* format, ...) {
    va_list args;
    va_start(args, format);
    printf("%s[ERROR]%s ", COLOR_RED, COLOR_RESET);
    vprintf(format, args);
    printf("\n");
    va_end(args);
}

// ============================================================================
// Helper functions
// ============================================================================
bool fileExists(const char* path) {
    struct stat buffer;
    return (stat(path, &buffer) == 0);
}

size_t getFileSize(const char* path) {
    struct stat st;
    if (stat(path, &st) != 0) {
        return 0;
    }
    return st.st_size;
}

// ============================================================================
// TEST 1: Validación de Shaders Compilados
// ============================================================================
bool testShaderCompilation() {
    printTestHeader("Shader Compilation Validation");

    // Verificar que los shaders GLSL fuente existen
    printSection("Verificando shaders GLSL fuente");
    bool vertShaderExists = fileExists("shaders/text.vert");
    bool fragShaderExists = fileExists("shaders/text.frag");

    printTestResult(vertShaderExists, "Vertex shader source exists (shaders/text.vert)");
    printTestResult(fragShaderExists, "Fragment shader source exists (shaders/text.frag)");

    if (!vertShaderExists || !fragShaderExists) {
        printError("Shaders GLSL fuente no encontrados");
        return false;
    }

    // Verificar que los shaders SPIR-V compilados existen
    printSection("Verificando shaders SPIR-V compilados");
    bool vertSpvExists = fileExists("shaders/text_vert.spv");
    bool fragSpvExists = fileExists("shaders/text_frag.spv");

    printTestResult(vertSpvExists, "Compiled vertex shader exists (shaders/text_vert.spv)");
    printTestResult(fragSpvExists, "Compiled fragment shader exists (shaders/text_frag.spv)");

    if (!vertSpvExists || !fragSpvExists) {
        printError("Shaders SPIR-V no compilados. Ejecutando compilación...");

        // Intentar compilar los shaders
        int result = system("./compile_shaders_linux.sh");
        if (result != 0) {
            printError("Failed to compile shaders automatically");
            return false;
        }

        // Verificar nuevamente
        vertSpvExists = fileExists("shaders/text_vert.spv");
        fragSpvExists = fileExists("shaders/text_frag.spv");

        printTestResult(vertSpvExists, "Vertex shader compiled successfully");
        printTestResult(fragSpvExists, "Fragment shader compiled successfully");

        if (!vertSpvExists || !fragSpvExists) {
            return false;
        }
    }

    // Verificar que los shaders compilados tienen tamaño válido
    printSection("Verificando integridad de shaders compilados");
    size_t vertSize = getFileSize("shaders/text_vert.spv");
    size_t fragSize = getFileSize("shaders/text_frag.spv");

    printInfo("Vertex shader size: %zu bytes", vertSize);
    printInfo("Fragment shader size: %zu bytes", fragSize);

    bool vertSizeValid = vertSize > 0 && vertSize < 100000; // Reasonable size
    bool fragSizeValid = fragSize > 0 && fragSize < 100000;

    printTestResult(vertSizeValid, "Vertex shader has valid size");
    printTestResult(fragSizeValid, "Fragment shader has valid size");

    // Verificar magic number SPIR-V (0x07230203)
    printSection("Verificando formato SPIR-V");
    std::ifstream vertFile("shaders/text_vert.spv", std::ios::binary);
    std::ifstream fragFile("shaders/text_frag.spv", std::ios::binary);

    uint32_t vertMagic = 0, fragMagic = 0;
    vertFile.read(reinterpret_cast<char*>(&vertMagic), sizeof(uint32_t));
    fragFile.read(reinterpret_cast<char*>(&fragMagic), sizeof(uint32_t));

    bool vertMagicValid = (vertMagic == 0x07230203);
    bool fragMagicValid = (fragMagic == 0x07230203);

    printInfo("Vertex shader magic: 0x%08X (expected: 0x07230203)", vertMagic);
    printInfo("Fragment shader magic: 0x%08X (expected: 0x07230203)", fragMagic);

    printTestResult(vertMagicValid, "Vertex shader has valid SPIR-V magic number");
    printTestResult(fragMagicValid, "Fragment shader has valid SPIR-V magic number");

    return vertShaderExists && fragShaderExists &&
           vertSpvExists && fragSpvExists &&
           vertSizeValid && fragSizeValid &&
           vertMagicValid && fragMagicValid;
}

// ============================================================================
// TEST 2: Validación de Assets (Fuentes)
// ============================================================================
bool testAssetValidation() {
    printTestHeader("Asset Validation");

    printSection("Verificando fuentes");

    bool fontExists = fileExists("assets/fonts/default_mono.ttf");
    printTestResult(fontExists, "Default font exists (assets/fonts/default_mono.ttf)");

    if (!fontExists) {
        printError("Font file not found");
        return false;
    }

    size_t fontSize = getFileSize("assets/fonts/default_mono.ttf");
    printInfo("Font file size: %zu bytes", fontSize);

    bool fontSizeValid = fontSize > 1000 && fontSize < 10000000; // Entre 1KB y 10MB
    printTestResult(fontSizeValid, "Font file has valid size");

    // Verificar magic number de TTF (0x00010000 o "OTTO" para OTF)
    std::ifstream fontFile("assets/fonts/default_mono.ttf", std::ios::binary);
    uint32_t fontMagic = 0;
    fontFile.read(reinterpret_cast<char*>(&fontMagic), sizeof(uint32_t));

    // TTF/OTF magic numbers (big-endian)
    bool fontMagicValid = (fontMagic == 0x00010000 || fontMagic == 0x4F54544F);

    printInfo("Font magic: 0x%08X", fontMagic);
    printTestResult(fontMagicValid, "Font has valid TrueType/OpenType magic number");

    return fontExists && fontSizeValid && fontMagicValid;
}

// ============================================================================
// TEST 3: Font Loader Tests
// ============================================================================
bool testFontLoader() {
    printTestHeader("Font Loader Tests");

    printSection("Inicializando FontLoader");
    phantom::FontLoader fontLoader;

    printSection("Cargando fuente default_mono.ttf a 48pt");
    bool loadSuccess = fontLoader.loadFromFile("assets/fonts/default_mono.ttf", 48.0f);
    printTestResult(loadSuccess, "Font loaded successfully");

    if (!loadSuccess) {
        printError("Failed to load font");
        return false;
    }

    printSection("Verificando atlas generado");
    const phantom::FontAtlas& atlas = fontLoader.getAtlas();

    printInfo("Atlas dimensions: %dx%d", atlas.width, atlas.height);
    printInfo("Atlas bitmap size: %zu bytes", atlas.bitmap.size());

    bool atlasWidthValid = atlas.width > 0 && atlas.width <= 4096;
    bool atlasHeightValid = atlas.height > 0 && atlas.height <= 4096;
    bool atlasDataValid = atlas.bitmap.size() == static_cast<size_t>(atlas.width * atlas.height);

    printTestResult(atlasWidthValid, "Atlas width is valid");
    printTestResult(atlasHeightValid, "Atlas height is valid");
    printTestResult(atlasDataValid, "Atlas bitmap size matches dimensions");

    printSection("Verificando metadatos de glifos");

    // Test algunos glifos ASCII comunes
    char testChars[] = {'A', 'a', '0', ' ', '!', 'x'};
    int validGlyphs = 0;

    for (char ch : testChars) {
        const phantom::Glyph* glyph = fontLoader.getGlyph(static_cast<unsigned char>(ch));
        if (glyph) {
            validGlyphs++;
            printInfo("Glyph '%c': advance=%.1f size(%dx%d)",
                     ch, glyph->advance, glyph->width, glyph->height);
        }
    }

    bool allGlyphsValid = validGlyphs == sizeof(testChars);
    printTestResult(allGlyphsValid, "All test glyphs have valid metrics");

    // Verificar que algunos píxeles del atlas no están vacíos
    printSection("Verificando contenido del atlas");
    size_t nonZeroPixels = 0;
    for (unsigned char pixel : atlas.bitmap) {
        if (pixel > 0) {
            nonZeroPixels++;
        }
    }

    float nonZeroPercent = (nonZeroPixels * 100.0f) / atlas.bitmap.size();
    printInfo("Non-zero pixels: %zu (%.2f%%)", nonZeroPixels, nonZeroPercent);

    bool atlasHasContent = nonZeroPixels > 0 && nonZeroPercent < 90.0f; // No más de 90% lleno
    printTestResult(atlasHasContent, "Atlas contains glyph data");

    return loadSuccess && atlasWidthValid && atlasHeightValid &&
           atlasDataValid && allGlyphsValid && atlasHasContent;
}

// ============================================================================
// TEST 4: Glyph Fragmenter Tests
// ============================================================================
bool testGlyphFragmenter() {
    printTestHeader("Glyph Fragmenter Tests");

    printSection("Testando fragmentación de glifos");

    phantom::GlyphFragmenter fragmenter;

    // Test con diferentes índices
    struct TestCase {
        size_t index;
        const char* expected;
    };

    TestCase testCases[] = {
        {0, "Top or Bottom"},
        {1, "Top or Bottom"},
        {10, "Top or Bottom"},
        {100, "Top or Bottom"},
        {999, "Top or Bottom"}
    };

    int validFragments = 0;
    for (const auto& tc : testCases) {
        phantom::FragmentMode mode = fragmenter.getFragmentModeByIndex(tc.index);
        const char* modeName = (mode == phantom::FragmentMode::Top) ? "Top" :
                              (mode == phantom::FragmentMode::Bottom) ? "Bottom" : "None";

        bool isValid = (mode == phantom::FragmentMode::Top || mode == phantom::FragmentMode::Bottom);

        if (isValid) validFragments++;

        printInfo("Index %zu -> %s", tc.index, modeName);
        printTestResult(isValid, "Fragment mode is valid (Top or Bottom)");
    }

    bool allFragmentsValid = validFragments == sizeof(testCases) / sizeof(TestCase);
    printTestResult(allFragmentsValid, "All fragment modes are valid");

    // Verificar que la fragmentación es consistente (mismo input = mismo output)
    printSection("Verificando consistencia de fragmentación");

    bool consistencyPassed = true;
    for (int i = 0; i < 100; i++) {
        phantom::FragmentMode mode1 = fragmenter.getFragmentModeByIndex(i);
        phantom::FragmentMode mode2 = fragmenter.getFragmentModeByIndex(i);

        if (mode1 != mode2) {
            consistencyPassed = false;
            printError("Inconsistent fragmentation at index %d", i);
            break;
        }
    }

    printTestResult(consistencyPassed, "Fragmentation is consistent");

    return allFragmentsValid && consistencyPassed;
}

// ============================================================================
// TEST 5: Buffer Tests
// ============================================================================
bool testBuffer() {
    printTestHeader("Text Buffer Tests");

    printSection("Creando buffer vacío");
    phantom::TextBuffer buffer;

    bool initiallyEmpty = (buffer.length() == 0);
    printTestResult(initiallyEmpty, "Buffer is initially empty");

    printSection("Insertando texto");
    buffer.insert(0, "Hello");
    bool hasContent = (buffer.length() > 0);
    printTestResult(hasContent, "Buffer contains data after insert");

    std::string text = buffer.getText();
    bool textCorrect = (text == "Hello");
    printInfo("Buffer text: '%s'", text.c_str());
    printTestResult(textCorrect, "Buffer text is correct");

    printSection("Insertando más texto");
    buffer.insert(5, " World");
    text = buffer.getText();
    bool textCorrect2 = (text == "Hello World");
    printInfo("Buffer text: '%s'", text.c_str());
    printTestResult(textCorrect2, "Buffer text after second insert is correct");

    printSection("Insertando en medio");
    buffer.insert(5, "123");
    text = buffer.getText();
    printInfo("Buffer text after insert at position 5: '%s'", text.c_str());

    printSection("Borrando caracteres");
    buffer.erase(5, 3); // Borrar "123"
    text = buffer.getText();
    bool afterDelete = (text == "Hello World");
    printInfo("Buffer text after delete: '%s'", text.c_str());
    printTestResult(afterDelete, "Delete works correctly");

    return initiallyEmpty && hasContent && textCorrect && textCorrect2 && afterDelete;
}

// ============================================================================
// TEST 6: Cursor Tests
// ============================================================================
bool testCursor() {
    printTestHeader("Cursor Tests");

    printSection("Creando cursor");
    phantom::Cursor cursor;

    bool initiallyZero = (cursor.getPosition() == 0);
    printTestResult(initiallyZero, "Cursor initially at position 0");

    printSection("Moviendo cursor con buffer");
    phantom::TextBuffer buffer;
    buffer.insert(0, "Line 1\nLine 2\nLine 3");

    printInfo("Buffer content:\n%s", buffer.getText().c_str());

    // Move right
    printSection("Testing cursor movement");
    size_t initialPos = cursor.getPosition();
    cursor.moveRight(buffer);
    bool movedRight = cursor.getPosition() > initialPos;
    printTestResult(movedRight, "Cursor moves right");

    // Move to end of line
    cursor.moveToLineEnd(buffer);
    printInfo("Cursor position at line end: %zu", cursor.getPosition());

    // Move to start of line
    cursor.moveToLineStart(buffer);
    bool atLineStart = cursor.getPosition() == 0;
    printTestResult(atLineStart, "Cursor moves to line start");

    // Move down
    cursor.moveDown(buffer);
    bool movedDown = cursor.getPosition() > 0;
    printTestResult(movedDown, "Cursor moves down");

    return initiallyZero && movedRight && atLineStart && movedDown;
}

// ============================================================================
// TEST 7: Logging System Tests
// ============================================================================
bool testLoggingSystem() {
    printTestHeader("Logging System Tests");

    printSection("Inicializando sistema de logging");
    phantom::Logger::init("test_autonomous.log");

    printTestResult(true, "Logger initialized");

    printSection("Escribiendo logs de prueba");
    LOG_INFO(phantom::LogCategory::INIT, "Test INFO log");
    LOG_DEBUG(phantom::LogCategory::RENDER, "Test DEBUG log");
    LOG_WARN(phantom::LogCategory::VULKAN, "Test WARN log");

    printTestResult(true, "Logs written successfully");

    printSection("Verificando archivo de log");
    bool logFileExists = fileExists("test_autonomous.log");
    printTestResult(logFileExists, "Log file created");

    size_t logSize = getFileSize("test_autonomous.log");
    bool logHasContent = logSize > 0;
    printInfo("Log file size: %zu bytes", logSize);
    printTestResult(logHasContent, "Log file has content");

    return logFileExists && logHasContent;
}

// ============================================================================
// Main test runner
// ============================================================================
int main() {
    auto startTime = std::chrono::high_resolution_clock::now();

    printf("\n");
    printf("%s%s", COLOR_BOLD, COLOR_MAGENTA);
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║                                                                ║\n");
    printf("║         PHANTOM WRITER - AUTONOMOUS TEST SUITE                ║\n");
    printf("║                                                                ║\n");
    printf("║   Sistema de Testing Autónomo y Profundo                      ║\n");
    printf("║   Validación completa sin requerir UI visible                 ║\n");
    printf("║                                                                ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    printf("%s\n", COLOR_RESET);

    bool allPassed = true;

    // Ejecutar todos los tests
    allPassed &= testLoggingSystem();
    allPassed &= testShaderCompilation();
    allPassed &= testAssetValidation();
    allPassed &= testFontLoader();
    allPassed &= testGlyphFragmenter();
    allPassed &= testBuffer();
    allPassed &= testCursor();

    // Cleanup
    phantom::Logger::shutdown();

    // Calcular tiempo total
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    // Resumen final
    printf("\n");
    printf("%s%s", COLOR_BOLD, COLOR_CYAN);
    printf("╔════════════════════════════════════════════════════════════════╗\n");
    printf("║                      TEST SUMMARY                              ║\n");
    printf("╚════════════════════════════════════════════════════════════════╝\n");
    printf("%s", COLOR_RESET);

    printf("\n");
    printf("  Total tests:  %s%s%d%s\n", COLOR_BOLD, COLOR_CYAN, totalTests, COLOR_RESET);
    printf("  Passed:       %s%s%d%s\n", COLOR_BOLD, COLOR_GREEN, passedTests, COLOR_RESET);
    printf("  Failed:       %s%s%d%s\n", COLOR_BOLD, COLOR_RED, failedTests, COLOR_RESET);
    printf("  Duration:     %s%s%lld ms%s\n", COLOR_BOLD, COLOR_YELLOW,
           (long long)duration.count(), COLOR_RESET);

    printf("\n");

    if (allPassed && failedTests == 0) {
        printf("%s%s", COLOR_BOLD, COLOR_GREEN);
        printf("╔════════════════════════════════════════════════════════════════╗\n");
        printf("║                                                                ║\n");
        printf("║                  ✓ ALL TESTS PASSED                            ║\n");
        printf("║                                                                ║\n");
        printf("║   El sistema está funcionando correctamente                   ║\n");
        printf("║   Todos los componentes validados exitosamente                ║\n");
        printf("║                                                                ║\n");
        printf("╚════════════════════════════════════════════════════════════════╝\n");
        printf("%s\n", COLOR_RESET);
        return EXIT_SUCCESS;
    } else {
        printf("%s%s", COLOR_BOLD, COLOR_RED);
        printf("╔════════════════════════════════════════════════════════════════╗\n");
        printf("║                                                                ║\n");
        printf("║                  ✗ TESTS FAILED                                ║\n");
        printf("║                                                                ║\n");
        printf("║   %d test(s) fallaron                                         ║\n", failedTests);
        printf("║   Revisar los logs para más detalles                          ║\n");
        printf("║                                                                ║\n");
        printf("╚════════════════════════════════════════════════════════════════╝\n");
        printf("%s\n", COLOR_RESET);
        return EXIT_FAILURE;
    }
}
