#include "platform/win32/window.h"

#include <cstdio>

int main() {
    orbis::win32::Window window;
    if (!window.create(L"Orbis", 1280, 720)) 
    {
        std::fprintf(stderr, "Failed to create window\n");
        return 1;
    }

    while (window.poll()) 
    {
        Sleep(1);
    }

    return 0;
}
