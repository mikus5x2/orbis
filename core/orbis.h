#pragma once

#include "core/orbis_canvas.h"

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

private:
    Canvas canvas_;
};

Context& createContext();
void destroyContext(Context* target = nullptr);
Context& context();

void beginFrame(int width, int height);

Canvas& canvas();

} // namespace orbis