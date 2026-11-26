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

} 

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

    // std::wprintf(L"Adapter:       %s\n", device.adapterName().c_str());
    // std::wprintf(L"Feature level: %s\n", featureLevelName(device.featureLevel()));
    // std::wprintf(L"Debug layer:   %s\n", device.debugLayerEnabled() ? L"on" : L"off");
    std::fflush(stdout);

    while (window.poll())
    {
        Sleep(1);
    }

    device.destroy();
    return 0;
}
