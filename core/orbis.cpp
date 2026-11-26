#include "core/orbis.h"

namespace orbis
{
namespace
{

Context* gContext = nullptr;

}

void Context::beginFrame(int width, int height)
{
    drawList_.reset(static_cast<float>(width), static_cast<float>(height));
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

DrawList& drawList()
{
    return context().drawList();
}

}