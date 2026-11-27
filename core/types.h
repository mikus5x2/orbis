// Orbis v0.1.0
// Lightweight immediate-mode UI framework
// https://github.com/mikus5x2/orbis
// Licensed under the MIT License. See LICENSE for details.

#pragma once

#include <cstdint>

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

} // namespace orbis