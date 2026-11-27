// Orbis v0.1.0
// Lightweight immediate-mode UI framework
// https://github.com/mikus5x2/orbis
// Licensed under the MIT License. See LICENSE for details.

#include "core/orbis_impl_font_internal.h"

#if defined(ORBIS_WITH_FREETYPE)

#include <cstring>

#include <ft2build.h>
#include FT_FREETYPE_H

namespace orbis
{

bool rasterizeFreeType(std::span<const std::uint8_t> data, float pixelHeight, RasterizedFont& result)
{
    FT_Library library = nullptr;
    if (FT_Init_FreeType(&library) != 0)
    {
        return false;
    }

    FT_Face face = nullptr;
    if (FT_New_Memory_Face(
            library, data.data(), static_cast<FT_Long>(data.size()), 0, &face) != 0)
    {
        FT_Done_FreeType(library);
        return false;
    }

    if (FT_Set_Char_Size(face, 0, static_cast<FT_F26Dot6>(pixelHeight * 64.0f), 72, 72) != 0)
    {
        FT_Done_Face(face);
        FT_Done_FreeType(library);
        return false;
    }

    result.ascent = static_cast<float>(face->size->metrics.ascender) / 64.0f;
    result.lineHeight = static_cast<float>(face->size->metrics.height) / 64.0f;

    for (int i = 0; i < Font::kGlyphCount; ++i)
    {
        const FT_ULong codepoint = static_cast<FT_ULong>(i + kFirstGlyph);
        if (FT_Load_Char(
                face,
                codepoint,
                FT_LOAD_RENDER | FT_LOAD_TARGET_LIGHT | FT_LOAD_NO_BITMAP) != 0)
        {
            continue;
        }

        const FT_GlyphSlot slot = face->glyph;
        const FT_Bitmap& bitmap = slot->bitmap;

        GlyphBitmap& glyph = result.glyphs[i];
        glyph.width = static_cast<int>(bitmap.width);
        glyph.height = static_cast<int>(bitmap.rows);
        glyph.pitch = static_cast<int>(bitmap.pitch);
        glyph.xOffset = static_cast<float>(slot->bitmap_left);
        glyph.yOffset = static_cast<float>(-slot->bitmap_top);
        glyph.advance = static_cast<float>(slot->advance.x) / 64.0f;

        if (glyph.width <= 0 || glyph.height <= 0)
        {
            continue;
        }

        glyph.offset = result.pixels.size();
        const std::size_t size = static_cast<std::size_t>(glyph.pitch) * glyph.height;
        result.pixels.resize(result.pixels.size() + size);
        std::memcpy(result.pixels.data() + glyph.offset, bitmap.buffer, size);
    }

    FT_Done_Face(face);
    FT_Done_FreeType(library);
    return true;
}

} // namespace orbis

#endif