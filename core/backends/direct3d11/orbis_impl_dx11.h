#pragma once

#include "core/orbis_draw.h"

#include <cstddef>
#include <string>

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <d3d11.h>
#include <d3dcompiler.h>
#include <wrl/client.h>

namespace orbis::d3d11
{

class Renderer
{
public:
    Renderer() = default;
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    bool init(ID3D11Device* device, ID3D11DeviceContext* context);
    void shutdown();

    void draw(const DrawList& drawList);

    const std::string& lastError() const { return lastError_; }

private:
    bool createShaders();
    bool createStates();
    bool createWhiteTexture();
    bool ensureBuffers(std::size_t vertexCount, std::size_t indexCount);
    bool compileShader(const char* source, const char* entry, const char* target, ID3DBlob** blob);

    ID3D11Device* device_ = nullptr;
    ID3D11DeviceContext* context_ = nullptr;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader_;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader_;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout_;
    Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer_;
    Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer_;
    Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer_;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> whiteTexture_;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler_;
    Microsoft::WRL::ComPtr<ID3D11BlendState> blendState_;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState_;

    std::size_t vertexCapacity_ = 0;
    std::size_t indexCapacity_ = 0;
    std::string lastError_;
};

} // namespace orbis::d3d11