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

bool Device::createSwapChain(HWND hwnd, int width, int height)
{
    if (!device_ || !adapter_ || !hwnd || width <= 0 || height <= 0)
    {
        return false;
    }

    Microsoft::WRL::ComPtr<IDXGIFactory2> factory;
    HRESULT hr = adapter_->GetParent(IID_PPV_ARGS(factory.GetAddressOf()));
    if (FAILED(hr))
    {
        lastError_ = hr;
        return false;
    }

    factory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);

    DXGI_SWAP_CHAIN_DESC1 desc{};
    desc.Width = static_cast<UINT>(width);
    desc.Height = static_cast<UINT>(height);
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.BufferCount = 2;
    desc.Scaling = DXGI_SCALING_STRETCH;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    desc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

    hr = factory->CreateSwapChainForHwnd(
        device_.Get(), hwnd, &desc, nullptr, nullptr, swapChain_.GetAddressOf());
    if (FAILED(hr))
    {
        lastError_ = hr;
        return false;
    }

    hr = createRenderTarget();
    if (FAILED(hr))
    {
        lastError_ = hr;
        return false;
    }

    width_ = width;
    height_ = height;
    return true;
}

HRESULT Device::createRenderTarget()
{
    Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
    HRESULT hr = swapChain_->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf()));
    if (FAILED(hr))
    {
        return hr;
    }

    return device_->CreateRenderTargetView(
        backBuffer.Get(), nullptr, renderTarget_.GetAddressOf());
}

void Device::beginFrame(float r, float g, float b, float a)
{
    if (!context_ || !renderTarget_)
    {
        return;
    }

    const float clearColor[4] = {r, g, b, a};
    context_->ClearRenderTargetView(renderTarget_.Get(), clearColor);

    ID3D11RenderTargetView* targets[] = {renderTarget_.Get()};
    context_->OMSetRenderTargets(1, targets, nullptr);

    D3D11_VIEWPORT viewport{};
    viewport.Width = static_cast<float>(width_);
    viewport.Height = static_cast<float>(height_);
    viewport.MaxDepth = 1.0f;
    context_->RSSetViewports(1, &viewport);
}

void Device::present()
{
    if (!swapChain_)
    {
        return;
    }

    swapChain_->Present(1, 0);
}

void Device::destroy()
{
    renderTarget_.Reset();
    swapChain_.Reset();
    adapter_.Reset();
    context_.Reset();
    device_.Reset();

    adapterName_.clear();
    featureLevel_ = D3D_FEATURE_LEVEL_11_0;
    lastError_ = S_OK;
    debugLayer_ = false;
    width_ = 0;
    height_ = 0;
}

} // namespace orbis::d3d11
