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
#include <dxgi.h>
#include <wrl/client.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

namespace orbis::d3d11
{

// no swapchain for now imma add it later.
class Device
{
public:
    Device() = default;
    ~Device();

    Device(const Device&) = delete;
    Device& operator=(const Device&) = delete;

    bool create();
    void destroy();

    ID3D11Device* device() const { return device_.Get(); }
    ID3D11DeviceContext* context() const { return context_.Get(); }
    IDXGIAdapter* adapter() const { return adapter_.Get(); }

    const std::wstring& adapterName() const { return adapterName_; }
    D3D_FEATURE_LEVEL featureLevel() const { return featureLevel_; }
    HRESULT lastError() const { return lastError_; }
    bool debugLayerEnabled() const { return debugLayer_; }

private:
    Microsoft::WRL::ComPtr<ID3D11Device> device_;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_;
    Microsoft::WRL::ComPtr<IDXGIAdapter> adapter_;

    std::wstring adapterName_;
    D3D_FEATURE_LEVEL featureLevel_ = D3D_FEATURE_LEVEL_11_0;
    HRESULT lastError_ = S_OK;
    bool debugLayer_ = false;
};

} // namespace orbis::d3d11
