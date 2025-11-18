#include "glyph_fragmenter.h"
#include "utils/logger.h"

namespace phantom {

GlyphFragmenter::GlyphFragmenter() {
    LOG_TRACE(LogCategory::RENDER, "GlyphFragmenter created");
}

GlyphFragmenter::~GlyphFragmenter() {
    LOG_TRACE(LogCategory::RENDER, "GlyphFragmenter destroyed");
}

FragmentMode GlyphFragmenter::getFragmentMode(size_t line, size_t column) const {
    uint32_t hash = hashPosition(line, column);

    // Use least significant bit to determine top or bottom
    // This gives a ~50/50 distribution
    FragmentMode mode = (hash & 1) ? FragmentMode::Bottom : FragmentMode::Top;

    LOG_TRACE(LogCategory::RENDER, "Glyph at (%zu, %zu): mode=%s",
        line, column, mode == FragmentMode::Top ? "TOP" : "BOTTOM");

    return mode;
}

FragmentMode GlyphFragmenter::getFragmentModeByIndex(size_t index) const {
    // Simple hash: multiply by large prime and use LSB
    uint32_t hash = static_cast<uint32_t>(index * 2654435761u);
    return (hash & 1) ? FragmentMode::Bottom : FragmentMode::Top;
}

uint32_t GlyphFragmenter::hashPosition(size_t line, size_t column) const {
    // FNV-1a hash variant for position
    // This ensures consistent results for same position
    uint32_t hash = 2166136261u; // FNV offset basis

    // Hash line
    hash ^= static_cast<uint32_t>(line);
    hash *= 16777619u; // FNV prime

    // Hash column
    hash ^= static_cast<uint32_t>(column);
    hash *= 16777619u;

    return hash;
}

} // namespace phantom
