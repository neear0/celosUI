#include "../win32/ui_win32.h"

// celosia::platform on top of the Win32 window and D3D11 device in this folder

namespace celosia::platform {
    static HWND hwnd = nullptr;
    static std::wstring title_wide; // the windows map keeps a pointer to the title

    bool create(const char* title, ImVec2 pos, ImVec2 size) {
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

    bool poll() {
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

    void new_frame() {
        celosia_win32::effect_renderer::update_backdrop();
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
    }

    void present() {
        const float clear_color_with_alpha[4] = { 0,0,0,0 };
        celosia_win32::d3d::device_context->OMSetRenderTargets(1, &celosia_win32::d3d::render_target_view, nullptr);
        celosia_win32::d3d::device_context->ClearRenderTargetView(celosia_win32::d3d::render_target_view, clear_color_with_alpha);

        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        celosia_win32::d3d::swapchain->Present(1, 0); // Present with vsync, (use 0, 0 for no vsync, although unnecessary because I don't think I'll be needing 1000+ fps for this. Maybe limit it to 60 fps or something even.)
        celosia_win32::d3d::report_debug_messages();
    }

    void destroy() {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        celosia_win32::effect_renderer::destroy();
        celosia_win32::d3d::destroy_device();
        celosia_win32::window::destroy();
        hwnd = nullptr;
    }

    bool key_held(int key) {
        return GetAsyncKeyState(key) & 0x8000;
    }

    ImVec2 cursor_pos() {
        POINT p;
        if (GetCursorPos(&p))
            return ImVec2(p.x, p.y);
        return ImVec2(0, 0);
    }

    ImRect window_rect() {
        RECT r;
        GetWindowRect(hwnd, &r);
        return ImRect(r.left, r.top, r.right, r.bottom);
    }

    ImVec2 screen_size() {
        return ImVec2(GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
    }

    void move(ImVec2 pos) {
        celosia_win32::window::move(hwnd, pos);
    }

    void bring_to_top() {
        BringWindowToTop(hwnd);
    }

    bool blur_supported() {
        return true;
    }

    void set_blur(bool enabled) { // 4 is acrylic blur behind the window, 0 turns it off
        celosia_win32::window::enable_blur(hwnd, enabled ? 4 : 0);
    }
}
