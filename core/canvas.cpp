// Orbis v0.1.0
// Lightweight immediate-mode UI framework
// https://github.com/mikus5x2/orbis
// Licensed under the MIT License. See LICENSE for details.

#include "core/canvas.h"

#include "core/font.h"

#include <algorithm>
#include <cmath>

namespace orbis
{
namespace
{

bool sameRect(const Rect& a, const Rect& b)
{
    return a.x == b.x && a.y == b.y && a.width == b.width && a.height == b.height;
}

bool outsideClip(const Rect& rect, const Rect& clip)
{
    return rect.x + rect.width <= clip.x
        || rect.y + rect.height <= clip.y
        || rect.x >= clip.x + clip.width
        || rect.y >= clip.y + clip.height;
}

}

void Canvas::reset(float width, float height, const Font& font)
{
    vertices_.clear();
    indices_.clear();
    commands_.clear();
    clipStack_.clear();

    width_ = width;
    height_ = height;
    font_ = &font;
    texture_ = font.texture;
    whiteUv_ = font.whiteUv();
    clipStack_.push_back({0.0f, 0.0f, width, height});
}

void Canvas::pushClipRect(const Rect& rect)
{
    Rect clip = rect;

    if (!clipStack_.empty())
    {
        const Rect& parent = clipStack_.back();
        const float x0 = std::max(clip.x, parent.x);
        const float y0 = std::max(clip.y, parent.y);
        const float x1 = std::min(clip.x + clip.width, parent.x + parent.width);
        const float y1 = std::min(clip.y + clip.height, parent.y + parent.height);

        clip = {x0, y0, std::max(x1 - x0, 0.0f), std::max(y1 - y0, 0.0f)};
    }

    clipStack_.push_back(clip);
}

void Canvas::popClipRect()
{
    if (clipStack_.size() > 1)
    {
        clipStack_.pop_back();
    }
}

void Canvas::fillRect(const Rect& rect, Color color)
{
    if (rect.width <= 0.0f || rect.height <= 0.0f || outsideClip(rect, clipStack_.back()))
    {
        return;
    }

    const std::size_t vertexStart = vertices_.size();
    const std::size_t indexStart = indices_.size();

    addCommand(static_cast<std::uint32_t>(indexStart));
    commands_.back().indexCount += 6;
    append(4, 6);

    const float x0 = rect.x;
    const float y0 = rect.y;
    const float x1 = rect.x + rect.width;
    const float y1 = rect.y + rect.height;

    Vertex* v = vertices_.data() + vertexStart;
    v[0] = {{x0, y0}, whiteUv_, color};
    v[1] = {{x1, y0}, whiteUv_, color};
    v[2] = {{x1, y1}, whiteUv_, color};
    v[3] = {{x0, y1}, whiteUv_, color};

    std::uint32_t* i = indices_.data() + indexStart;
    const auto base = static_cast<std::uint32_t>(vertexStart);
    i[0] = base + 0;
    i[1] = base + 1;
    i[2] = base + 2;
    i[3] = base + 0;
    i[4] = base + 2;
    i[5] = base + 3;
}

void Canvas::text(Vec2 position, std::string_view text, Color color)
{
    emitText(position, text, color);
}

void Canvas::textOutlined(
    Vec2 position,
    std::string_view text,
    Color color,
    Color outline,
    float thickness)
{
    if (text.empty() || (color.value >> 24) == 0)
    {
        return;
    }

    if ((outline.value >> 24) != 0 && thickness > 0.0f)
    {
        emitText({position.x - thickness, position.y}, text, outline);
        emitText({position.x + thickness, position.y}, text, outline);
        emitText({position.x, position.y - thickness}, text, outline);
        emitText({position.x, position.y + thickness}, text, outline);
    }

    emitText(position, text, color);
}

void Canvas::emitText(Vec2 position, std::string_view text, Color color)
{
    if (font_ == nullptr || text.empty())
    {
        return;
    }

    const std::size_t vertexStart = vertices_.size();
    const std::size_t indexStart = indices_.size();
    append(
        static_cast<std::uint32_t>(text.size() * 4),
        static_cast<std::uint32_t>(text.size() * 6));

    Vertex* vertex = vertices_.data() + vertexStart;
    std::uint32_t* index = indices_.data() + indexStart;
    const auto base = static_cast<std::uint32_t>(vertexStart);
    std::uint32_t vertexCount = 0;
    std::uint32_t indexCount = 0;

    const Rect& clip = clipStack_.back();
    const float startX = std::floor(position.x + 0.5f);
    float x = startX;
    float y = std::floor(position.y + font_->ascent() + 0.5f);

    for (const char codepoint : text)
    {
        if (codepoint == '\n')
        {
            x = startX;
            y = std::floor(y + font_->lineHeight() + 0.5f);
            continue;
        }

        const Glyph* glyph = font_->glyph(codepoint);
        if (glyph == nullptr)
        {
            continue;
        }

        if (glyph->width > 0.0f)
        {
            const Rect quad{
                x + glyph->xOffset,
                y + glyph->yOffset,
                glyph->width,
                glyph->height};

            if (!outsideClip(quad, clip))
            {
                const float x1 = quad.x + quad.width;
                const float y1 = quad.y + quad.height;

                vertex[0] = {{quad.x, quad.y}, {glyph->u0, glyph->v0}, color};
                vertex[1] = {{x1, quad.y}, {glyph->u1, glyph->v0}, color};
                vertex[2] = {{x1, y1}, {glyph->u1, glyph->v1}, color};
                vertex[3] = {{quad.x, y1}, {glyph->u0, glyph->v1}, color};

                index[0] = base + vertexCount + 0;
                index[1] = base + vertexCount + 1;
                index[2] = base + vertexCount + 2;
                index[3] = base + vertexCount + 0;
                index[4] = base + vertexCount + 2;
                index[5] = base + vertexCount + 3;

                vertex += 4;
                index += 6;
                vertexCount += 4;
                indexCount += 6;
            }
        }

        x += glyph->advance;
    }

    vertices_.resize(vertexStart + vertexCount);
    indices_.resize(indexStart + indexCount);

    if (indexCount > 0)
    {
        addCommand(static_cast<std::uint32_t>(indexStart));
        commands_.back().indexCount += indexCount;
    }
}

void Canvas::addCommand(std::uint32_t indexOffset)
{
    const Rect& clip = clipStack_.back();

    if (!commands_.empty())
    {
        DrawCommand& last = commands_.back();
        if (last.texture == texture_ && sameRect(last.clip, clip))
        {
            return;
        }
    }

    DrawCommand command;
    command.clip = clip;
    command.texture = texture_;
    command.indexOffset = indexOffset;
    commands_.push_back(command);
}

void Canvas::append(std::uint32_t vertexCount, std::uint32_t indexCount)
{
    vertices_.resize(vertices_.size() + vertexCount);
    indices_.resize(indices_.size() + indexCount);
}

} // namespace orbis