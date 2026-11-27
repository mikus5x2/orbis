// Orbis v0.1.0
// Lightweight immediate-mode UI framework
// https://github.com/mikus5x2/orbis
// Licensed under the MIT License. See LICENSE for details.

#include "core/backends/direct3d11/orbis_impl_dx11.h"

#include "core/orbis_impl_font.h"

#include <algorithm>
#include <cstring>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

namespace orbis::d3d11
{
namespace
{

constexpr const char* kShaderSource = R"(
cbuffer Constants : register(b0)
{
    float2 viewportSize;
    float2 padding;
};

struct VSInput
{
    float2 position : POSITION;
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
};

struct PSInput
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
    float4 color : COLOR0;
};

PSInput vsMain(VSInput input)
{
    PSInput output;
    output.position = float4(
        input.position.x / viewportSize.x * 2.0f - 1.0f,
        1.0f - input.position.y / viewportSize.y * 2.0f,
        0.0f,
        1.0f);
    output.uv = input.uv;
    output.color = input.color;
    return output;
}

Texture2D uiTexture : register(t0);
SamplerState uiSampler : register(s0);

float4 psMain(PSInput input) : SV_Target
{
    const float coverage = uiTexture.Sample(uiSampler, input.uv).r;
    return float4(input.color.rgb, input.color.a * coverage);
}
)";

} 

Renderer::~Renderer()
{
    shutdown();
}

bool Renderer::init(ID3D11Device* device, ID3D11DeviceContext* context)
{
    if (!device || !context)
    {
        return false;
    }

    shutdown();
    lastError_.clear();

    device_ = device;
    context_ = context;

    if (!createShaders() || !createStates() || !createWhiteTexture())
    {
        shutdown();
        return false;
    }

    return true;
}

void Renderer::shutdown()
{
    rasterizerState_.Reset();
    blendState_.Reset();
    sampler_.Reset();
    textures_.clear();
    indexBuffer_.Reset();
    vertexBuffer_.Reset();
    constantBuffer_.Reset();
    inputLayout_.Reset();
    pixelShader_.Reset();
    vertexShader_.Reset();

    device_ = nullptr;
    context_ = nullptr;
    vertexCapacity_ = 0;
    indexCapacity_ = 0;
}

void Renderer::draw(const Canvas& canvas)
{
    const float width = canvas.width();
    const float height = canvas.height();

    if (!context_ || textures_.empty() || width <= 0.0f || height <= 0.0f
        || canvas.commands().empty())
    {
        return;
    }

    if (!ensureBuffers(canvas.vertices().size(), canvas.indices().size()))
    {
        return;
    }

    D3D11_MAPPED_SUBRESOURCE mapped{};

    if (FAILED(context_->Map(vertexBuffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
    {
        return;
    }
    std::memcpy(mapped.pData, canvas.vertices().data(), canvas.vertices().size() * sizeof(Vertex));
    context_->Unmap(vertexBuffer_.Get(), 0);

    if (FAILED(context_->Map(indexBuffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
    {
        return;
    }
    std::memcpy(mapped.pData, canvas.indices().data(), canvas.indices().size() * sizeof(std::uint32_t));
    context_->Unmap(indexBuffer_.Get(), 0);

    struct Constants
    {
        float width;
        float height;
        float padding[2];
    };

    if (SUCCEEDED(context_->Map(constantBuffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
    {
        const Constants constants{width, height, {0.0f, 0.0f}};
        std::memcpy(mapped.pData, &constants, sizeof(constants));
        context_->Unmap(constantBuffer_.Get(), 0);
    }

    D3D11_VIEWPORT viewport{};
    viewport.Width = width;
    viewport.Height = height;
    viewport.MaxDepth = 1.0f;
    context_->RSSetViewports(1, &viewport);

    const UINT stride = sizeof(Vertex);
    const UINT offset = 0;
    ID3D11Buffer* vertexBuffers[] = {vertexBuffer_.Get()};
    ID3D11Buffer* constantBuffers[] = {constantBuffer_.Get()};
    ID3D11SamplerState* samplers[] = {sampler_.Get()};

    context_->IASetInputLayout(inputLayout_.Get());
    context_->IASetVertexBuffers(0, 1, vertexBuffers, &stride, &offset);
    context_->IASetIndexBuffer(indexBuffer_.Get(), DXGI_FORMAT_R32_UINT, 0);
    context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context_->VSSetShader(vertexShader_.Get(), nullptr, 0);
    context_->PSSetShader(pixelShader_.Get(), nullptr, 0);
    context_->VSSetConstantBuffers(0, 1, constantBuffers);
    context_->PSSetSamplers(0, 1, samplers);
    context_->OMSetBlendState(blendState_.Get(), nullptr, 0xFFFFFFFF);
    context_->RSSetState(rasterizerState_.Get());

    const LONG maxX = static_cast<LONG>(width);
    const LONG maxY = static_cast<LONG>(height);

    for (const DrawCommand& command : canvas.commands())
    {
        D3D11_RECT scissor{
            std::clamp(static_cast<LONG>(command.clip.x), 0L, maxX),
            std::clamp(static_cast<LONG>(command.clip.y), 0L, maxY),
            std::clamp(static_cast<LONG>(command.clip.x + command.clip.width), 0L, maxX),
            std::clamp(static_cast<LONG>(command.clip.y + command.clip.height), 0L, maxY)};

        if (scissor.right <= scissor.left || scissor.bottom <= scissor.top)
        {
            continue;
        }

        ID3D11ShaderResourceView* view = textures_[0].Get();
        if (command.texture < textures_.size())
        {
            view = textures_[command.texture].Get();
        }

        context_->PSSetShaderResources(0, 1, &view);
        context_->RSSetScissorRects(1, &scissor);
        context_->DrawIndexed(command.indexCount, command.indexOffset, 0);
    }

    ID3D11ShaderResourceView* none = nullptr;
    context_->PSSetShaderResources(0, 1, &none);
}

bool Renderer::createShaders()
{
    Microsoft::WRL::ComPtr<ID3DBlob> vertexBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> pixelBlob;

    if (!compileShader(kShaderSource, "vsMain", "vs_5_0", vertexBlob.GetAddressOf()))
    {
        return false;
    }

    if (!compileShader(kShaderSource, "psMain", "ps_5_0", pixelBlob.GetAddressOf()))
    {
        return false;
    }

    if (FAILED(device_->CreateVertexShader(
            vertexBlob->GetBufferPointer(),
            vertexBlob->GetBufferSize(),
            nullptr,
            vertexShader_.GetAddressOf())))
    {
        return false;
    }

    if (FAILED(device_->CreatePixelShader(
            pixelBlob->GetBufferPointer(),
            pixelBlob->GetBufferSize(),
            nullptr,
            pixelShader_.GetAddressOf())))
    {
        return false;
    }

    const D3D11_INPUT_ELEMENT_DESC layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Vertex, position), D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, offsetof(Vertex, uv), D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, offsetof(Vertex, color), D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    return SUCCEEDED(device_->CreateInputLayout(
        layout,
        _countof(layout),
        vertexBlob->GetBufferPointer(),
        vertexBlob->GetBufferSize(),
        inputLayout_.GetAddressOf()));
}

bool Renderer::createStates()
{
    D3D11_BUFFER_DESC constantDesc{};
    constantDesc.ByteWidth = 16;
    constantDesc.Usage = D3D11_USAGE_DYNAMIC;
    constantDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constantDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    if (FAILED(device_->CreateBuffer(&constantDesc, nullptr, constantBuffer_.GetAddressOf())))
    {
        return false;
    }

    D3D11_SAMPLER_DESC samplerDesc{};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    if (FAILED(device_->CreateSamplerState(&samplerDesc, sampler_.GetAddressOf())))
    {
        return false;
    }

    D3D11_BLEND_DESC blendDesc{};
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    if (FAILED(device_->CreateBlendState(&blendDesc, blendState_.GetAddressOf())))
    {
        return false;
    }

    D3D11_RASTERIZER_DESC rasterizerDesc{};
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;
    rasterizerDesc.CullMode = D3D11_CULL_NONE;
    rasterizerDesc.FrontCounterClockwise = FALSE;
    rasterizerDesc.DepthClipEnable = TRUE;
    rasterizerDesc.ScissorEnable = TRUE;

    return SUCCEEDED(device_->CreateRasterizerState(&rasterizerDesc, rasterizerState_.GetAddressOf()));
}

bool Renderer::createWhiteTexture()
{
    const std::uint8_t pixel = 255;

    D3D11_TEXTURE2D_DESC textureDesc{};
    textureDesc.Width = 1;
    textureDesc.Height = 1;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R8_UNORM;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA data{};
    data.pSysMem = &pixel;
    data.SysMemPitch = 1;

    Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
    if (FAILED(device_->CreateTexture2D(&textureDesc, &data, texture.GetAddressOf())))
    {
        return false;
    }

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> view;
    if (FAILED(device_->CreateShaderResourceView(texture.Get(), nullptr, view.GetAddressOf())))
    {
        return false;
    }

    textures_.push_back(view);
    return true;
}

bool Renderer::createFontTexture(Font& font)
{
    const FontAtlas atlas = font.atlas();

    if (!device_ || atlas.pixels == nullptr || atlas.width <= 0 || atlas.height <= 0)
    {
        return false;
    }

    D3D11_TEXTURE2D_DESC textureDesc{};
    textureDesc.Width = static_cast<UINT>(atlas.width);
    textureDesc.Height = static_cast<UINT>(atlas.height);
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R8_UNORM;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA data{};
    data.pSysMem = atlas.pixels;
    data.SysMemPitch = static_cast<UINT>(atlas.width);

    Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
    if (FAILED(device_->CreateTexture2D(&textureDesc, &data, texture.GetAddressOf())))
    {
        return false;
    }

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> view;
    if (FAILED(device_->CreateShaderResourceView(texture.Get(), nullptr, view.GetAddressOf())))
    {
        return false;
    }

    textures_.push_back(view);
    font.texture = static_cast<TextureId>(textures_.size() - 1);
    return true;
}

bool Renderer::ensureBuffers(std::size_t vertexCount, std::size_t indexCount)
{
    if (vertexCount > vertexCapacity_)
    {
        vertexCapacity_ = vertexCount * 2;

        D3D11_BUFFER_DESC desc{};
        desc.ByteWidth = static_cast<UINT>(vertexCapacity_ * sizeof(Vertex));
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        if (FAILED(device_->CreateBuffer(&desc, nullptr, vertexBuffer_.ReleaseAndGetAddressOf())))
        {
            vertexCapacity_ = 0;
            return false;
        }
    }

    if (indexCount > indexCapacity_)
    {
        indexCapacity_ = indexCount * 2;

        D3D11_BUFFER_DESC desc{};
        desc.ByteWidth = static_cast<UINT>(indexCapacity_ * sizeof(std::uint32_t));
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        if (FAILED(device_->CreateBuffer(&desc, nullptr, indexBuffer_.ReleaseAndGetAddressOf())))
        {
            indexCapacity_ = 0;
            return false;
        }
    }

    return true;
}

bool Renderer::compileShader(const char* source, const char* entry, const char* target, ID3DBlob** blob)
{
    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(_DEBUG)
    flags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    Microsoft::WRL::ComPtr<ID3DBlob> errors;
    HRESULT hr = D3DCompile(
        source,
        std::strlen(source),
        nullptr,
        nullptr,
        nullptr,
        entry,
        target,
        flags,
        0,
        blob,
        errors.GetAddressOf());

    if (FAILED(hr))
    {
        if (errors)
        {
            lastError_.assign(
                static_cast<const char*>(errors->GetBufferPointer()),
                errors->GetBufferSize());
        }
        return false;
    }

    return true;
}

} // namespace orbis::d3d11