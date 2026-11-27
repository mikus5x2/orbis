// Orbis v0.1.0
// Lightweight immediate-mode UI framework
// https://github.com/mikus5x2/orbis
// Licensed under the MIT License. See LICENSE for details.

// Orbis v0.1.0
// Lightweight immediate-mode UI framework
// https://github.com/mikus5x2/orbis
// Licensed under the MIT License. See LICENSE for details.

#pragma once

#include "core/font.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace orbis
{

constexpr int kFirstGlyph = 32;

struct GlyphBitmap
{
    std::size_t offset = 0;
    int width = 0;
    int height = 0;
    int pitch = 0;
    float xOffset = 0.0f;
    float yOffset = 0.0f;
    float advance = 0.0f;
};

struct RasterizedFont
{
    std::vector<std::uint8_t> pixels;
    std::array<GlyphBitmap, Font::kGlyphCount> glyphs{};
    float ascent = 0.0f;
    float lineHeight = 0.0f;
};

bool rasterizeStb(std::span<const std::uint8_t> data, float pixelHeight, RasterizedFont& result);
bool rasterizeFreeType(std::span<const std::uint8_t> data, float pixelHeight, RasterizedFont& result);

} // namespace orbis
