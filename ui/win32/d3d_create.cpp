#include "../win32/ui_win32.h"

namespace celosia_win32::d3d {
    bool create_device(HWND hwnd) {
        DXGI_SWAP_CHAIN_DESC sd;
        ZeroMemory(&sd, sizeof(sd));
        sd.BufferCount = 2;
        sd.BufferDesc.Width = 0;
        sd.BufferDesc.Height = 0;
        sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        sd.BufferDesc.RefreshRate.Numerator = 60;
        sd.BufferDesc.RefreshRate.Denominator = 1;
        sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
        sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        sd.OutputWindow = hwnd;
        sd.SampleDesc.Count = 1;
        sd.SampleDesc.Quality = 0;
        sd.Windowed = TRUE;
        sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

        UINT create_device_flags = 0;
        D3D_FEATURE_LEVEL feature_level;
        const D3D_FEATURE_LEVEL feature_level_array[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };

        HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, create_device_flags, feature_level_array, 2, D3D11_SDK_VERSION, &sd, &d3d::swapchain, &d3d::device, &feature_level, &d3d::device_context);
        if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
            res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, create_device_flags, feature_level_array, 2, D3D11_SDK_VERSION, &sd, &d3d::swapchain, &d3d::device, &feature_level, &d3d::device_context);
        if (res != S_OK)
            return false;

        ID3D11Texture2D* back_buffer;
        d3d::swapchain->GetBuffer(0, IID_PPV_ARGS(&back_buffer));
        d3d::device->CreateRenderTargetView(back_buffer, nullptr, &d3d::render_target_view);
        back_buffer->Release();
        return true;
    }
}