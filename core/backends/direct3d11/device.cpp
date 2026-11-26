#include "core/backends/direct3d11/device.h"

namespace orbis::d3d11
{
namespace
{

HRESULT createRawDevice(UINT flags, ID3D11Device** device, D3D_FEATURE_LEVEL* level,
                        ID3D11DeviceContext** context)
{
    constexpr D3D_FEATURE_LEVEL kLevel = D3D_FEATURE_LEVEL_11_0;

    return D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        flags,
        &kLevel,
        1,
        D3D11_SDK_VERSION,
        device,
        level,
        context);
}

} // namespace

Device::~Device()
{
    destroy();
}

bool Device::create()
{
    destroy();

    UINT flags = 0;
#if defined(_DEBUG)
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    HRESULT hr = createRawDevice(
        flags, device_.GetAddressOf(), &featureLevel_, context_.GetAddressOf());

#if defined(_DEBUG)
    if (FAILED(hr) && (flags & D3D11_CREATE_DEVICE_DEBUG))
    {
        flags &= ~D3D11_CREATE_DEVICE_DEBUG;
        hr = createRawDevice(
            flags, device_.GetAddressOf(), &featureLevel_, context_.GetAddressOf());
    }
#endif

    if (FAILED(hr))
    {
        destroy();
        lastError_ = hr;
        return false;
    }

    debugLayer_ = (flags & D3D11_CREATE_DEVICE_DEBUG) != 0;

    Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
    if (SUCCEEDED(device_.As(&dxgiDevice)))
    {
        if (SUCCEEDED(dxgiDevice->GetAdapter(adapter_.GetAddressOf())))
        {
            DXGI_ADAPTER_DESC desc{};
            if (SUCCEEDED(adapter_->GetDesc(&desc)))
            {
                adapterName_ = desc.Description;
            }
        }
    }

    lastError_ = S_OK;
    return true;
}

void Device::destroy()
{
    adapter_.Reset();
    context_.Reset();
    device_.Reset();

    adapterName_.clear();
    featureLevel_ = D3D_FEATURE_LEVEL_11_0;
    lastError_ = S_OK;
    debugLayer_ = false;
}

} // namespace orbis::d3d11
