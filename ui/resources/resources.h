/*
#pragma once

#include "../../external/imgui/imgui.h"

namespace celosia::resources {

    inline ID3D11Texture2D* tex = nullptr;
    void load_texture(std::string file_path) {

        D3D11_TEXTURE2D_DESC image_texture_desc = {};

        image_texture_desc.Width = 400;
        image_texture_desc.Height = 400;
        image_texture_desc.MipLevels = 1;
        image_texture_desc.ArraySize = 1;
        image_texture_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        image_texture_desc.SampleDesc.Count = 1;
        image_texture_desc.SampleDesc.Quality = 0;
        image_texture_desc.Usage = D3D11_USAGE_IMMUTABLE;
        image_texture_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

        D3D11_SUBRESOURCE_DATA image_subresource_data = {};

        image_subresource_data.pSysMem = image_data;
        image_subresource_data.SysMemPitch = image_pitch;


       // celosia::d3d::device->CreateTexture2D(&image_texture_desc, 0, &tex);
    }
}
*/