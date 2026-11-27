// Orbis v0.1.0
// Lightweight immediate-mode UI framework
// https://github.com/mikus/orbis
// Licensed under the MIT License. See LICENSE for details.

#include "core/orbis_font.h"

#include <cstring>
#include <fstream>

#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

namespace orbis
{
namespace
{

constexpr int kFirstGlyph = 32;

} 

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

    return bake(data.data(), pixelHeight);
}

bool Font::bake(const void* data, float pixelHeight)
{
    const auto* bytes = static_cast<const std::uint8_t*>(data);

    stbtt_fontinfo info{};
    if (stbtt_InitFont(&info, bytes, 0) == 0)
    {
        return false;
    }

    int width = 512;
    int height = 512;
    std::array<stbtt_packedchar, kGlyphCount> packed{};

    for (;;)
    {
        pixels_.assign(static_cast<std::size_t>(width) * height, 0);

        stbtt_pack_context context{};
        if (stbtt_PackBegin(&context, pixels_.data(), width, height - 1, 0, 1, nullptr) == 0)
        {
            return false;
        }

        const int result = stbtt_PackFontRange(
            &context, bytes, 0, pixelHeight, kFirstGlyph, kGlyphCount, packed.data());
        stbtt_PackEnd(&context);

        if (result != 0)
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

    std::memset(pixels_.data() + static_cast<std::size_t>(height - 1) * width, 255, width);

    width_ = width;
    height_ = height;
    whiteUv_ = {0.5f / width, (height - 0.5f) / height};

    const float scale = stbtt_ScaleForPixelHeight(&info, pixelHeight);
    int ascent = 0;
    int descent = 0;
    int lineGap = 0;
    stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);

    ascent_ = ascent * scale;
    lineHeight_ = (ascent - descent + lineGap) * scale;

    for (int i = 0; i < kGlyphCount; ++i)
    {
        const stbtt_packedchar& source = packed[i];
        Glyph& glyph = glyphs_[i];

        glyph.u0 = source.x0 / static_cast<float>(width);
        glyph.v0 = source.y0 / static_cast<float>(height);
        glyph.u1 = source.x1 / static_cast<float>(width);
        glyph.v1 = source.y1 / static_cast<float>(height);
        glyph.xOffset = source.xoff;
        glyph.yOffset = source.yoff;
        glyph.advance = source.xadvance;
        glyph.width = static_cast<float>(source.x1 - source.x0);
        glyph.height = static_cast<float>(source.y1 - source.y0);
    }

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
