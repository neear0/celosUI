#include "../win32/ui_win32.h"

#include "shaders/gradient_dxbc.h"
#include "shaders/glow_dxbc.h"
#include "shaders/blur_dxbc.h"
#include "shaders/background_dxbc.h"

// d3d11 side of celosia::effects: every effect is a pixel shader that the draw list callback swaps in,
// ImGui's vertex shader, input layout and blending stay as they are

namespace celosia_win32::effect_renderer {
    static ID3D11PixelShader* shaders[(int)celosia::effects::e_type::count] = {};
    static ID3D11Buffer* constant_buffer = nullptr;
    static ID3D11SamplerState* clamp_sampler = nullptr; // the blur reads near the frame's edges, ImGui's sampler wraps around
    static ID3D11Texture2D* backdrop = nullptr;           // copy of the back buffer the blur reads from
    static ID3D11ShaderResourceView* backdrop_view = nullptr;

    bool create() {
        struct { const BYTE* code; SIZE_T size; } code[] = { // same order as effects::e_type
            { gradient_dxbc, sizeof(gradient_dxbc) },
            { glow_dxbc, sizeof(glow_dxbc) },
            { blur_dxbc, sizeof(blur_dxbc) },
            { background_dxbc, sizeof(background_dxbc) },
        };
        static_assert(std::size(code) == (size_t)celosia::effects::e_type::count);
        for (int i = 0; i < (int)celosia::effects::e_type::count; i++)
            if (FAILED(d3d::device->CreatePixelShader(code[i].code, code[i].size, nullptr, &shaders[i])))
                return false;

        D3D11_BUFFER_DESC buffer = {};
        buffer.ByteWidth = sizeof(celosia::effects::t_constants);
        buffer.Usage = D3D11_USAGE_DYNAMIC;
        buffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        buffer.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        if (FAILED(d3d::device->CreateBuffer(&buffer, nullptr, &constant_buffer)))
            return false;

        D3D11_SAMPLER_DESC sampler = {};
        sampler.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        sampler.AddressU = sampler.AddressV = sampler.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
        sampler.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
        sampler.MaxLOD = D3D11_FLOAT32_MAX;
        if (FAILED(d3d::device->CreateSamplerState(&sampler, &clamp_sampler)))
            return false;

        update_backdrop();
        return backdrop_view != nullptr;
    }

    void update_backdrop() {
        ID3D11Texture2D* back_buffer = nullptr;
        if (FAILED(d3d::swapchain->GetBuffer(0, IID_PPV_ARGS(&back_buffer))))
            return;
        D3D11_TEXTURE2D_DESC desc;
        back_buffer->GetDesc(&desc);
        back_buffer->Release();

        if (backdrop) {
            D3D11_TEXTURE2D_DESC current;
            backdrop->GetDesc(&current);
            if (current.Width == desc.Width && current.Height == desc.Height && current.Format == desc.Format)
                return;
            backdrop_view->Release(); backdrop_view = nullptr;
            backdrop->Release(); backdrop = nullptr;
        }

        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags = 0;
        desc.MiscFlags = 0;
        if (FAILED(d3d::device->CreateTexture2D(&desc, nullptr, &backdrop)))
            return;
        if (FAILED(d3d::device->CreateShaderResourceView(backdrop, nullptr, &backdrop_view))) {
            backdrop->Release();
            backdrop = nullptr;
        }
    }

    void destroy() {
        for (ID3D11PixelShader*& shader : shaders)
            if (shader) { shader->Release(); shader = nullptr; }
        if (constant_buffer) { constant_buffer->Release(); constant_buffer = nullptr; }
        if (clamp_sampler) { clamp_sampler->Release(); clamp_sampler = nullptr; }
        if (backdrop_view) { backdrop_view->Release(); backdrop_view = nullptr; }
        if (backdrop) { backdrop->Release(); backdrop = nullptr; }
    }

    static void copy_frame() { // what's been drawn so far, for the blur to read
        ID3D11Resource* back_buffer = nullptr;
        d3d::render_target_view->GetResource(&back_buffer);
        d3d::device_context->CopyResource(backdrop, back_buffer);
        back_buffer->Release();
    }

    static void bind(const celosia::effects::t_command& effect) {
        ID3D11DeviceContext* context = d3d::device_context;
        if (effect.type == celosia::effects::e_type::blur) {
            copy_frame();
            context->PSSetSamplers(0, 1, &clamp_sampler); // ImGui puts its own sampler back when the effect ends
        }

        D3D11_MAPPED_SUBRESOURCE mapped;
        if (SUCCEEDED(context->Map(constant_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
            memcpy(mapped.pData, &effect.constants, sizeof(effect.constants));
            context->Unmap(constant_buffer, 0);
        }
        context->PSSetConstantBuffers(0, 1, &constant_buffer);
        context->PSSetShader(shaders[(int)effect.type], nullptr, 0);
    }
}

namespace celosia::platform {
    void draw_effect(const ImDrawList*, const ImDrawCmd* command) {
        celosia_win32::effect_renderer::bind(effects::commands[(size_t)(intptr_t)command->UserCallbackData]);
    }

    ImTextureID backdrop_texture() {
        return (ImTextureID)celosia_win32::effect_renderer::backdrop_view;
    }
}
