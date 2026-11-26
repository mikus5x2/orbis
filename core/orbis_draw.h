#pragma once

#include <cstdint>
#include <vector>

namespace orbis
{

using TextureId = std::uint32_t;

struct Vec2
{
    float x = 0.0f;
    float y = 0.0f;
};

struct Rect
{
    float x = 0.0f;
    float y = 0.0f;
    float width = 0.0f;
    float height = 0.0f;
};

struct Color
{
    std::uint32_t value = 0xFF000000;
};

constexpr Color rgba(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a = 255)
{
    return Color{
        static_cast<std::uint32_t>(r)
        | (static_cast<std::uint32_t>(g) << 8)
        | (static_cast<std::uint32_t>(b) << 16)
        | (static_cast<std::uint32_t>(a) << 24)};
}

struct Vertex
{
    Vec2 position;
    Vec2 uv;
    Color color;
};

struct DrawCommand
{
    Rect clip;
    TextureId texture = 0;
    std::uint32_t indexOffset = 0;
    std::uint32_t indexCount = 0;
};

class DrawList
{
public:
    void reset(float width, float height);

    void pushClipRect(const Rect& rect);
    void popClipRect();
    void setTexture(TextureId texture);

    void addRectFilled(const Rect& rect, Color color);

    float width() const { return width_; }
    float height() const { return height_; }

    const std::vector<Vertex>& vertices() const { return vertices_; }
    const std::vector<std::uint32_t>& indices() const { return indices_; }
    const std::vector<DrawCommand>& commands() const { return commands_; }

private:
    void addCommand();
    void reserve(std::uint32_t vertexCount, std::uint32_t indexCount);

    std::vector<Vertex> vertices_;
    std::vector<std::uint32_t> indices_;
    std::vector<DrawCommand> commands_;
    std::vector<Rect> clipStack_;

    TextureId texture_ = 0;
    float width_ = 0.0f;
    float height_ = 0.0f;
};

} // namespace orbis