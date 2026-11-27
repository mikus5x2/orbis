// Orbis v0.1.0
// Lightweight immediate-mode UI framework
// https://github.com/mikus5x2/orbis
// Licensed under the MIT License. See LICENSE for details.

#include "core/orbis_impl_font.h"

#include "core/orbis_impl_font_internal.h"

#include <cstring>
#include <fstream>

#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"

namespace orbis
{

bool Font::loadFromFile(const char* path, float pixelHeight)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file)
    {
        return false;
    }

    const std::streamsize size = file.tellg();
    if (size <= 0)
    {
        return false;
    }

    std::vector<std::uint8_t> data(static_cast<std::size_t>(size));
    file.seekg(0, std::ios::beg);

    if (!file.read(reinterpret_cast<char*>(data.data()), size))
    {
        return false;
    }

    RasterizedFont rasterized;

#if defined(ORBIS_WITH_FREETYPE)
    if (!rasterizeFreeType(data, pixelHeight, rasterized))
#else
    if (!rasterizeStb(data, pixelHeight, rasterized))
#endif
    {
        return false;
    }

    return buildAtlas(rasterized);
}

bool Font::buildAtlas(const RasterizedFont& rasterized)
{
    int width = 512;
    int height = 512;
    std::array<stbrp_rect, kGlyphCount> rects{};
    std::vector<stbrp_node> nodes;

    for (;;)
    {
        pixels_.assign(static_cast<std::size_t>(width) * height, 0);

        for (int i = 0; i < kGlyphCount; ++i)
        {
            rects[i].id = i;
            rects[i].w = rasterized.glyphs[i].width + 2;
            rects[i].h = rasterized.glyphs[i].height + 2;
        }

        nodes.resize(static_cast<std::size_t>(width));

        stbrp_context context{};
        stbrp_init_target(&context, width, height - 1, nodes.data(), width);

        if (stbrp_pack_rects(&context, rects.data(), kGlyphCount) != 0)
        {
            break;
        }

        if (width >= 4096)
        {
            return false;
        }

        width *= 2;
        height *= 2;
    }

    width_ = width;
    height_ = height;
    whiteUv_ = {0.5f / width, (height - 0.5f) / height};
    ascent_ = rasterized.ascent;
    lineHeight_ = rasterized.lineHeight;

    for (int i = 0; i < kGlyphCount; ++i)
    {
        const GlyphBitmap& source = rasterized.glyphs[i];
        const int x = rects[i].x + 1;
        const int y = rects[i].y + 1;

        for (int row = 0; row < source.height; ++row)
        {
            std::memcpy(
                pixels_.data() + static_cast<std::size_t>(y + row) * width + x,
                rasterized.pixels.data() + source.offset
                    + static_cast<std::size_t>(row) * source.pitch,
                static_cast<std::size_t>(source.width));
        }

        Glyph& glyph = glyphs_[i];
        glyph.u0 = static_cast<float>(x) / width;
        glyph.v0 = static_cast<float>(y) / height;
        glyph.u1 = static_cast<float>(x + source.width) / width;
        glyph.v1 = static_cast<float>(y + source.height) / height;
        glyph.xOffset = source.xOffset;
        glyph.yOffset = source.yOffset;
        glyph.advance = source.advance;
        glyph.width = static_cast<float>(source.width);
        glyph.height = static_cast<float>(source.height);
    }

    std::memset(pixels_.data() + static_cast<std::size_t>(height - 1) * width, 255, width);

    return true;
}

const Glyph* Font::glyph(char codepoint) const
{
    const int index = static_cast<unsigned char>(codepoint) - kFirstGlyph;
    if (index < 0 || index >= kGlyphCount)
    {
        return nullptr;
    }

    return &glyphs_[index];
}

float Font::measure(std::string_view text) const
{
    float widest = 0.0f;
    float x = 0.0f;

    for (const char codepoint : text)
    {
        if (codepoint == '\n')
        {
            widest = x > widest ? x : widest;
            x = 0.0f;
            continue;
        }

        const Glyph* found = glyph(codepoint);
        if (found != nullptr)
        {
            x += found->advance;
        }
    }

    return x > widest ? x : widest;
}

} // namespace orbis