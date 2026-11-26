#include "core/backends/direct3d11/device.h"
#include "platform/win32/window.h"

#include <cstdio>

namespace
{

const wchar_t* featureLevelName(D3D_FEATURE_LEVEL level)
{
    switch (level)
    {
    case D3D_FEATURE_LEVEL_11_1: return L"11_1";
    case D3D_FEATURE_LEVEL_11_0: return L"11_0";
    default: return L"unknown";
    }
}

} // namespace

int main()
{
    orbis::win32::enableDpiAwareness();

    orbis::win32::Window window;
    if (!window.create(L"Orbis", 1280, 720))
    {
        std::fprintf(stderr, "Failed to create window\n");
        return 1;
    }

    orbis::d3d11::Device device;
    if (!device.create())
    {
        std::fprintf(
            stderr,
            "Failed to create D3D11 device (HRESULT 0x%08lX)\n",
            static_cast<unsigned long>(device.lastError()));
        return 1;
    }

    if (!device.createSwapChain(window.handle(), window.width(), window.height()))
    {
        std::fprintf(
            stderr,
            "Failed to create swapchain (HRESULT 0x%08lX)\n",
            static_cast<unsigned long>(device.lastError()));
        return 1;
    }

    while (window.poll())
    {
        device.beginFrame(0.616f, 0.780f, 0.890f, 1.0f);
        device.present();
    }

    device.destroy();
    return 0;
}
