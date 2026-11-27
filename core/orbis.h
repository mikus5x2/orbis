// Orbis v0.1.0
// Lightweight immediate-mode UI framework
// https://github.com/mikus5x2/orbis
// Licensed under the MIT License. See LICENSE for details.

#pragma once

#include "core/canvas.h"
#include "core/font.h"

namespace orbis
{

class Context
{
public:
    Context() = default;
    Context(const Context&) = delete;
    Context& operator=(const Context&) = delete;

    void beginFrame(int width, int height);

    Canvas& canvas() { return canvas_; }
    const Canvas& canvas() const { return canvas_; }

    Font& font() { return font_; }
    const Font& font() const { return font_; }

private:
    Canvas canvas_;
    Font font_;
};

Context& createContext();
void destroyContext();
Context& context();

void beginFrame(int width, int height);

Canvas& canvas();

} // namespace orbis
