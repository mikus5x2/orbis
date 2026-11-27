// Orbis v0.1.0
// Lightweight immediate-mode UI framework
// https://github.com/mikus/orbis
// Licensed under the MIT License. See LICENSE for details.

#include "core/orbis.h"

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

void destroyContext(Context* target)
{
    if (target == nullptr)
    {
        target = gContext;
    }

    if (target == gContext)
    {
        gContext = nullptr;
    }

    delete target;
}

Context& context()
{
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

}