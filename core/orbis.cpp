// Orbis v0.1.0
// Lightweight immediate-mode UI framework
// https://github.com/mikus5x2/orbis
// Licensed under the MIT License. See LICENSE for details.

#include "core/orbis.h"

#include <cassert>

namespace orbis
{
namespace
{

Context* gContext = nullptr;

}

void Context::beginFrame(int width, int height)
{
    canvas_.reset(static_cast<float>(width), static_cast<float>(height), font_);
}

Context& createContext()
{
    destroyContext();

    gContext = new Context();
    return *gContext;
}

void destroyContext()
{
    delete gContext;
    gContext = nullptr;
}

Context& context()
{
    assert(gContext != nullptr);
    return *gContext;
}

void beginFrame(int width, int height)
{
    context().beginFrame(width, height);
}

Canvas& canvas()
{
    return context().canvas();
}

} // namespace orbis
