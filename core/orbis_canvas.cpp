#include "core/orbis_canvas.h"

#include <algorithm>

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

void Canvas::reset(float width, float height)
{
    vertices_.clear();
    indices_.clear();
    commands_.clear();
    clipStack_.clear();

    width_ = width;
    height_ = height;
    texture_ = 0;
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

    addCommand();
    commands_.back().indexCount += 6;

    const std::size_t vertexStart = vertices_.size();
    const std::size_t indexStart = indices_.size();
    reserve(4, 6);

    const float x0 = rect.x;
    const float y0 = rect.y;
    const float x1 = rect.x + rect.width;
    const float y1 = rect.y + rect.height;

    Vertex* v = vertices_.data() + vertexStart;
    v[0] = {{x0, y0}, {0.0f, 0.0f}, color};
    v[1] = {{x1, y0}, {1.0f, 0.0f}, color};
    v[2] = {{x1, y1}, {1.0f, 1.0f}, color};
    v[3] = {{x0, y1}, {0.0f, 1.0f}, color};

    std::uint32_t* i = indices_.data() + indexStart;
    const auto base = static_cast<std::uint32_t>(vertexStart);
    i[0] = base + 0;
    i[1] = base + 1;
    i[2] = base + 2;
    i[3] = base + 0;
    i[4] = base + 2;
    i[5] = base + 3;
}

void Canvas::addCommand()
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
    command.indexOffset = static_cast<std::uint32_t>(indices_.size());
    commands_.push_back(command);
}

void Canvas::reserve(std::uint32_t vertexCount, std::uint32_t indexCount)
{
    vertices_.resize(vertices_.size() + vertexCount);
    indices_.resize(indices_.size() + indexCount);
}

}