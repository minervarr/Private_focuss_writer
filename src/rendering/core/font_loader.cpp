#include "font_loader.h"
#include "utils/logger.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

#include <fstream>
#include <cmath>

namespace phantom {

FontLoader::FontLoader() {
    LOG_TRACE(LogCategory::RENDER, "FontLoader constructor");
}

FontLoader::~FontLoader() {
    LOG_TRACE(LogCategory::RENDER, "FontLoader destructor");
}

bool FontLoader::loadFromFile(const std::string& fontPath, float fontSize) {
    LOG_INFO(LogCategory::RENDER, "Loading font: %s (size: %.1f)", fontPath.c_str(), fontSize);

    // Read font file
    std::ifstream file(fontPath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        LOG_ERROR(LogCategory::RENDER, "Failed to open font file: %s", fontPath.c_str());
        return false;
    }

    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    fontFileData_.resize(fileSize);
    if (!file.read(reinterpret_cast<char*>(fontFileData_.data()), fileSize)) {
        LOG_ERROR(LogCategory::RENDER, "Failed to read font file: %s", fontPath.c_str());
        return false;
    }

    file.close();
    LOG_DEBUG(LogCategory::RENDER, "Font file loaded: %zu bytes", fontFileData_.size());

    // Generate atlas
    if (!generateAtlas(fontSize)) {
        LOG_ERROR(LogCategory::RENDER, "Failed to generate font atlas");
        return false;
    }

    LOG_INFO(LogCategory::RENDER, "Font loaded successfully: %zu glyphs, atlas %dx%d",
        atlas_.glyphs.size(), atlas_.width, atlas_.height);

    return true;
}

bool FontLoader::generateAtlas(float fontSize) {
    LOG_DEBUG(LogCategory::RENDER, "Generating font atlas (size: %.1f)", fontSize);

    stbtt_fontinfo font;
    if (!stbtt_InitFont(&font, fontFileData_.data(), 0)) {
        LOG_ERROR(LogCategory::RENDER, "Failed to initialize font");
        return false;
    }

    // Calculate scale for desired font size
    float scale = stbtt_ScaleForPixelHeight(&font, fontSize);

    // Get font metrics
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&font, &ascent, &descent, &lineGap);

    atlas_.fontSize = fontSize;
    atlas_.lineHeight = (ascent - descent + lineGap) * scale;

    LOG_DEBUG(LogCategory::RENDER, "Font scale: %.4f, line height: %.2f", scale, atlas_.lineHeight);

    // We'll pack ASCII printable characters (32-126) = 95 characters
    const int firstChar = 32;
    const int lastChar = 126;
    const int numChars = lastChar - firstChar + 1;

    // First pass: calculate atlas size
    int totalWidth = 0;
    int maxHeight = 0;

    std::vector<int> widths(numChars);
    std::vector<int> heights(numChars);

    for (int i = 0; i < numChars; i++) {
        int codepoint = firstChar + i;
        int width, height, xOffset, yOffset;

        stbtt_GetCodepointBitmapBox(&font, codepoint, scale, scale,
            &xOffset, &yOffset, &width, &height);

        width = width - xOffset;
        height = height - yOffset;

        widths[i] = width;
        heights[i] = height;

        totalWidth += width + 2; // 2 pixels padding
        maxHeight = std::max(maxHeight, height);
    }

    // Simple atlas layout: single row
    // For better packing, we could use multiple rows, but for FASE 1B we keep it simple
    atlas_.width = totalWidth;
    atlas_.height = maxHeight + 4; // Extra padding

    // Round up to power of 2 for better GPU compatibility
    auto nextPowerOf2 = [](int n) {
        n--;
        n |= n >> 1;
        n |= n >> 2;
        n |= n >> 4;
        n |= n >> 8;
        n |= n >> 16;
        n++;
        return n;
    };

    atlas_.width = nextPowerOf2(atlas_.width);
    atlas_.height = nextPowerOf2(atlas_.height);

    LOG_DEBUG(LogCategory::RENDER, "Atlas dimensions: %dx%d", atlas_.width, atlas_.height);

    // Allocate atlas bitmap (initialized to 0)
    atlas_.bitmap.resize(atlas_.width * atlas_.height, 0);

    // Second pass: rasterize glyphs into atlas
    int xOffset = 2; // Start with padding

    for (int i = 0; i < numChars; i++) {
        int codepoint = firstChar + i;

        int width, height, xoff, yoff;
        u8* glyphBitmap = stbtt_GetCodepointBitmap(&font, 0, scale,
            codepoint, &width, &height, &xoff, &yoff);

        if (!glyphBitmap) {
            LOG_WARN(LogCategory::RENDER, "Failed to rasterize glyph: %c (%d)", (char)codepoint, codepoint);
            continue;
        }

        // Copy glyph to atlas
        int yStart = 2; // Vertical padding
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int atlasX = xOffset + x;
                int atlasY = yStart + y;

                if (atlasX < atlas_.width && atlasY < atlas_.height) {
                    atlas_.bitmap[atlasY * atlas_.width + atlasX] = glyphBitmap[y * width + x];
                }
            }
        }

        // Get advance width
        int advance, leftSideBearing;
        stbtt_GetCodepointHMetrics(&font, codepoint, &advance, &leftSideBearing);

        // Create glyph metadata
        Glyph glyph;
        glyph.codepoint = codepoint;

        // Normalized texture coordinates
        glyph.x0 = static_cast<float>(xOffset) / atlas_.width;
        glyph.y0 = static_cast<float>(yStart) / atlas_.height;
        glyph.x1 = static_cast<float>(xOffset + width) / atlas_.width;
        glyph.y1 = static_cast<float>(yStart + height) / atlas_.height;

        glyph.xOffset = xoff;
        glyph.yOffset = yoff;
        glyph.advance = advance * scale;
        glyph.width = width;
        glyph.height = height;

        atlas_.glyphs.push_back(glyph);

        LOG_TRACE(LogCategory::RENDER, "Glyph '%c': pos(%d,%d), size(%dx%d), advance(%.2f)",
            (char)codepoint, xOffset, yStart, width, height, glyph.advance);

        stbtt_FreeBitmap(glyphBitmap, nullptr);

        xOffset += width + 2; // Move to next position with padding
    }

    LOG_INFO(LogCategory::RENDER, "Atlas generated: %zu glyphs", atlas_.glyphs.size());

    return true;
}

const Glyph* FontLoader::getGlyph(u32 codepoint) const {
    for (const auto& glyph : atlas_.glyphs) {
        if (glyph.codepoint == codepoint) {
            return &glyph;
        }
    }
    return nullptr;
}

} // namespace phantom
