#ifndef PHANTOM_FONT_LOADER_H
#define PHANTOM_FONT_LOADER_H

#include <phantom_writer/types.h>
#include <string>
#include <vector>

namespace phantom {

struct Glyph {
    u32 codepoint;       // Character code
    float x0, y0;        // Top-left position in atlas (normalized 0-1)
    float x1, y1;        // Bottom-right position in atlas (normalized 0-1)
    float xOffset;       // X offset when drawing
    float yOffset;       // Y offset when drawing
    float advance;       // Horizontal advance
    int width;           // Glyph width in pixels
    int height;          // Glyph height in pixels
};

struct FontAtlas {
    std::vector<u8> bitmap;      // Atlas bitmap data (grayscale)
    int width;                    // Atlas width
    int height;                   // Atlas height
    std::vector<Glyph> glyphs;   // Glyph metadata
    float fontSize;               // Font size in pixels
    float lineHeight;             // Recommended line spacing
};

class FontLoader {
public:
    FontLoader();
    ~FontLoader();

    // Load a TrueType font from file and generate atlas
    bool loadFromFile(const std::string& fontPath, float fontSize);

    // Get the generated atlas
    const FontAtlas& getAtlas() const { return atlas_; }

    // Get glyph for a specific character
    const Glyph* getGlyph(u32 codepoint) const;

private:
    FontAtlas atlas_;
    std::vector<u8> fontFileData_;

    bool generateAtlas(float fontSize);
};

} // namespace phantom

#endif // PHANTOM_FONT_LOADER_H
