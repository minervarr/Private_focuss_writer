#ifndef PHANTOM_GLYPH_FRAGMENTER_H
#define PHANTOM_GLYPH_FRAGMENTER_H

#include <cstdint>
#include <cstddef>

namespace phantom {

enum class FragmentMode : uint32_t {
    Top = 0,     // Show top half of glyph
    Bottom = 1,  // Show bottom half of glyph
    None = 2     // Show full glyph (no fragmentation)
};

class GlyphFragmenter {
public:
    GlyphFragmenter();
    ~GlyphFragmenter();

    // Determine fragment mode based on position
    // Uses a consistent hash to ensure same position always gets same mode
    FragmentMode getFragmentMode(size_t line, size_t column) const;

    // Alternative: get fragment mode by character index
    FragmentMode getFragmentModeByIndex(size_t index) const;

private:
    // Simple hash function for consistent fragmentation
    uint32_t hashPosition(size_t line, size_t column) const;
};

} // namespace phantom

#endif // PHANTOM_GLYPH_FRAGMENTER_H
