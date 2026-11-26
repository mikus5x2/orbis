#pragma once

#include <string>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

namespace orbis::d3d11
{

class Device
{
public:
    Device() = default;
    ~Device();

    Device(const Device&) = delete;
    Device& operator=(const Device&) = delete;

    bool create();
    bool createSwapChain(HWND hwnd, int width, int height);
    void destroy();

    void beginFrame(float r, float g, float b, float a);
    void present();

    ID3D11Device* device() const { return device_.Get(); }
    ID3D11DeviceContext* context() const { return context_.Get(); }
    IDXGIAdapter* adapter() const { return adapter_.Get(); }
    IDXGISwapChain1* swapChain() const { return swapChain_.Get(); }
    ID3D11RenderTargetView* renderTarget() const { return renderTarget_.Get(); }

    const std::wstring& adapterName() const { return adapterName_; }
    D3D_FEATURE_LEVEL featureLevel() const { return featureLevel_; }
    HRESULT lastError() const { return lastError_; }
    bool debugLayerEnabled() const { return debugLayer_; }
    int width() const { return width_; }
    int height() const { return height_; }

private:
    HRESULT createRenderTarget();

    Microsoft::WRL::ComPtr<ID3D11Device> device_;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_;
    Microsoft::WRL::ComPtr<IDXGIAdapter> adapter_;
    Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain_;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTarget_;

    std::wstring adapterName_;
    D3D_FEATURE_LEVEL featureLevel_ = D3D_FEATURE_LEVEL_11_0;
    HRESULT lastError_ = S_OK;
    bool debugLayer_ = false;
    int width_ = 0;
    int height_ = 0;
};

} // namespace orbis::d3d11
