// Orbis v0.1.0
// Lightweight immediate-mode UI framework
// https://github.com/mikus/orbis
// Licensed under the MIT License. See LICENSE for details.

#pragma once

#include "core/orbis_canvas.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>

namespace orbis
{

struct Glyph
{
    float u0 = 0.0f;
    float v0 = 0.0f;
    float u1 = 0.0f;
    float v1 = 0.0f;
    float xOffset = 0.0f;
    float yOffset = 0.0f;
    float advance = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
};

class Font
{
public:
    static constexpr int kGlyphCount = 95;

    bool loadFromFile(const char* path, float pixelHeight);

    const Glyph* glyph(char codepoint) const;
    float measure(std::string_view text) const;

    float ascent() const { return ascent_; }
    float lineHeight() const { return lineHeight_; }

    const std::uint8_t* pixels() const { return pixels_.data(); }
    int width() const { return width_; }
    int height() const { return height_; }
    Vec2 whiteUv() const { return whiteUv_; }

    TextureId texture = 0;

private:
    bool bake(const void* data, float pixelHeight);

    std::vector<std::uint8_t> pixels_;
    std::array<Glyph, kGlyphCount> glyphs_{};
    int width_ = 0;
    int height_ = 0;
    float ascent_ = 0.0f;
    float lineHeight_ = 0.0f;
    Vec2 whiteUv_{0.5f, 0.5f};
};

} // namespace orbis
