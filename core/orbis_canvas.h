// Orbis v0.1.0
// Lightweight immediate-mode UI framework
// https://github.com/mikus/orbis
// Licensed under the MIT License. See LICENSE for details.

#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

namespace orbis
{

using TextureId = std::uint32_t;

class Font;

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

    constexpr Color() = default;

    constexpr Color(std::uint8_t r, std::uint8_t g, std::uint8_t b, std::uint8_t a = 255)
        : value(
              static_cast<std::uint32_t>(r)
              | (static_cast<std::uint32_t>(g) << 8)
              | (static_cast<std::uint32_t>(b) << 16)
              | (static_cast<std::uint32_t>(a) << 24))
    {
    }
};

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

class Canvas
{
public:
    void reset(float width, float height, const Font& font);

    template <typename Fn>
    void clip(const Rect& rect, Fn&& body)
    {
        pushClipRect(rect);
        body();
        popClipRect();
    }

    template <typename Fn>
    void texture(TextureId id, Fn&& body)
    {
        const TextureId previous = texture_;
        texture_ = id;
        body();
        texture_ = previous;
    }

    void fillRect(const Rect& rect, Color color);
    void text(Vec2 position, std::string_view text, Color color);
    void textOutlined(
        Vec2 position,
        std::string_view text,
        Color color,
        Color outline,
        float thickness = 1.0f);

    float width() const { return width_; }
    float height() const { return height_; }

    const std::vector<Vertex>& vertices() const { return vertices_; }
    const std::vector<std::uint32_t>& indices() const { return indices_; }
    const std::vector<DrawCommand>& commands() const { return commands_; }

private:
    void pushClipRect(const Rect& rect);
    void popClipRect();
    void emitText(Vec2 position, std::string_view text, Color color);
    void addCommand(std::uint32_t indexOffset);
    void reserve(std::uint32_t vertexCount, std::uint32_t indexCount);

    std::vector<Vertex> vertices_;
    std::vector<std::uint32_t> indices_;
    std::vector<DrawCommand> commands_;
    std::vector<Rect> clipStack_;

    TextureId texture_ = 0;
    const Font* font_ = nullptr;
    Vec2 whiteUv_{0.5f, 0.5f};
    float width_ = 0.0f;
    float height_ = 0.0f;
};

} // namespace orbis
