#include "core/backends/direct3d11/device.h"
#include "core/backends/direct3d11/orbis_impl_dx11.h"
#include "core/orbis.h"
#include "platform/win32/window.h"

#include <array>
#include <cstdio>

int main()
{
    orbis::win32::enableDpiAwareness();

    orbis::win32::Window window;
    if (!window.create(L"Orbis DirectX 11 Example", 1366, 768))
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

    orbis::d3d11::Renderer renderer;
    if (!renderer.init(device.device(), device.context()))
    {
        std::fprintf(stderr, "Failed to init renderer: %s\n", renderer.lastError().c_str());
        return 1;
    }

    orbis::createContext();

    window.onResize = [&device](int width, int height)
    {
        device.resize(width, height);
    };

    while (window.poll())
    {
        orbis::beginFrame(device.width(), device.height());
        orbis::Canvas& canvas = orbis::canvas();
        const float w = canvas.width();
        const float h = canvas.height();
        canvas.fillRect({w / 2 - 16.0f, h / 2 - 16.0f, 32.0f, 32.0f}, {255, 0, 0});

        ID3D11RenderTargetView* renderTarget = device.renderTarget();
        device.context()->OMSetRenderTargets(1, &renderTarget, nullptr);
        device.context()->ClearRenderTargetView(renderTarget, std::array{0.08f, 0.09f, 0.11f, 1.0f}.data());

        renderer.draw(orbis::canvas());
        device.swapChain()->Present(1, 0);
    }

    orbis::destroyContext();
    renderer.shutdown();
    device.destroy();
    return 0;
}