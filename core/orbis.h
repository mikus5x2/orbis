#pragma once

#include "core/orbis_draw.h"

namespace orbis
{

class Context
{
public:
    Context() = default;
    Context(const Context&) = delete;
    Context& operator=(const Context&) = delete;

    void beginFrame(int width, int height);

    DrawList& drawList() { return drawList_; }
    const DrawList& drawList() const { return drawList_; }

private:
    DrawList drawList_;
};

Context& createContext();
void destroyContext(Context* target = nullptr);
Context& context();

void beginFrame(int width, int height);

DrawList& drawList();

} // namespace orbis