#include "../win32/ui_win32.h"

// celosia::platform on top of the Win32 window and D3D11 device in this folder

static HWND hwnd = nullptr;
static std::wstring title_wide; // the windows map keeps a pointer to the title

bool celosia::platform::create(const char* title, ImVec2 pos, ImVec2 size) {
    title_wide.resize(MultiByteToWideChar(CP_UTF8, 0, title, -1, nullptr, 0));
    MultiByteToWideChar(CP_UTF8, 0, title, -1, title_wide.data(), (int)title_wide.size());

    hwnd = celosia_win32::window::create(title_wide.c_str(), pos, size, WS_EX_TOPMOST | WS_EX_LAYERED);
    if (!hwnd || !celosia_win32::d3d::create_device(hwnd))
        return false;

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(celosia_win32::d3d::device, celosia_win32::d3d::device_context);
    if (!celosia_win32::effect_renderer::create())
        return false;

    celosia_win32::window::enable_transparency(hwnd);
    celosia_win32::window::enable_blur(hwnd, style::window::background == style::window::blur ? 4 : 0);
    celosia_win32::window::show(hwnd);
    return true;
}

bool celosia::platform::poll() {
    MSG msg;
    bool running = true;
    while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        if (msg.message == WM_QUIT)
            running = false;
    }
    return running;
}

void celosia::platform::new_frame() {
    celosia_win32::effect_renderer::update_backdrop();
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
}

void celosia::platform::present() {
    const float clear_color_with_alpha[4] = { 0,0,0,0 };
    celosia_win32::d3d::device_context->OMSetRenderTargets(1, &celosia_win32::d3d::render_target_view, nullptr);
    celosia_win32::d3d::device_context->ClearRenderTargetView(celosia_win32::d3d::render_target_view, clear_color_with_alpha);

    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    celosia_win32::d3d::swapchain->Present(1, 0); // Present with vsync, (use 0, 0 for no vsync, although unnecessary because I don't think I'll be needing 1000+ fps for this. Maybe limit it to 60 fps or something even.)
    celosia_win32::d3d::report_debug_messages();
}

void celosia::platform::destroy() {
    resources::images::clear();
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    celosia_win32::effect_renderer::destroy();
    celosia_win32::d3d::destroy_device();
    celosia_win32::window::destroy();
    hwnd = nullptr;
}

bool celosia::platform::key_held(int key) {
    return GetAsyncKeyState(key) & 0x8000;
}

ImVec2 celosia::platform::cursor_pos() {
    POINT p;
    if (GetCursorPos(&p))
        return ImVec2((float)p.x, (float)p.y);
    return ImVec2(0, 0);
}

ImRect celosia::platform::window_rect() {
    RECT r;
    GetWindowRect(hwnd, &r);
    return ImRect((float)r.left, (float)r.top, (float)r.right, (float)r.bottom);
}

ImVec2 celosia::platform::screen_size() {
    return ImVec2((float)GetSystemMetrics(SM_CXSCREEN), (float)GetSystemMetrics(SM_CYSCREEN));
}

void celosia::platform::move(ImVec2 pos) {
    celosia_win32::window::move(hwnd, pos);
}

void celosia::platform::bring_to_top() {
    BringWindowToTop(hwnd);
}

bool celosia::platform::blur_supported() {
    return true;
}

void celosia::platform::set_blur(bool enabled) { // 4 is acrylic blur behind the window, 0 turns it off
    celosia_win32::window::enable_blur(hwnd, enabled ? 4 : 0);
}

ImTextureID celosia::platform::create_texture(const unsigned char* rgba, int width, int height) { // the ImTextureID is the shader resource view, as ImGui's dx11 backend expects
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    D3D11_SUBRESOURCE_DATA data = { rgba, (UINT)width * 4, 0 };

    ID3D11Texture2D* texture = nullptr;
    if (FAILED(celosia_win32::d3d::device->CreateTexture2D(&desc, &data, &texture)))
        return nullptr;
    ID3D11ShaderResourceView* view = nullptr;
    celosia_win32::d3d::device->CreateShaderResourceView(texture, nullptr, &view);
    texture->Release(); // the view keeps it alive
    return (ImTextureID)view;
}

void celosia::platform::destroy_texture(ImTextureID texture) {
    if (texture)
        ((ID3D11ShaderResourceView*)texture)->Release();
}
