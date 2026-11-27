// Orbis v0.1.0
// Lightweight immediate-mode UI framework
// https://github.com/mikus5x2/orbis
// Licensed under the MIT License. See LICENSE for details.

#include "core/orbis_impl_font_internal.h"

#if !defined(ORBIS_WITH_FREETYPE)

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

namespace orbis
{

bool rasterizeStb(std::span<const std::uint8_t> data, float pixelHeight, RasterizedFont& result)
{
    stbtt_fontinfo info{};
    if (stbtt_InitFont(&info, data.data(), 0) == 0)
    {
        return false;
    }

    const float scale = stbtt_ScaleForPixelHeight(&info, pixelHeight);

    int ascent = 0;
    int descent = 0;
    int lineGap = 0;
    stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);

    result.ascent = ascent * scale;
    result.lineHeight = (ascent - descent + lineGap) * scale;

    for (int i = 0; i < Font::kGlyphCount; ++i)
    {
        const int codepoint = i + kFirstGlyph;
        GlyphBitmap& glyph = result.glyphs[i];

        int advance = 0;
        int leftSideBearing = 0;
        stbtt_GetCodepointHMetrics(&info, codepoint, &advance, &leftSideBearing);

        int x0 = 0;
        int y0 = 0;
        int x1 = 0;
        int y1 = 0;
        stbtt_GetCodepointBitmapBox(&info, codepoint, scale, scale, &x0, &y0, &x1, &y1);

        glyph.width = x1 - x0;
        glyph.height = y1 - y0;
        glyph.pitch = glyph.width;
        glyph.xOffset = static_cast<float>(x0);
        glyph.yOffset = static_cast<float>(y0);
        glyph.advance = advance * scale;

        if (glyph.width <= 0 || glyph.height <= 0)
        {
            continue;
        }

        glyph.offset = result.pixels.size();
        result.pixels.resize(
            result.pixels.size() + static_cast<std::size_t>(glyph.width) * glyph.height);

        stbtt_MakeCodepointBitmap(
            &info,
            result.pixels.data() + glyph.offset,
            glyph.width,
            glyph.height,
            glyph.pitch,
            scale,
            scale,
            codepoint);
    }

    return true;
}

} // namespace orbis

#endif