#include "../win32/ui_win32.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param);

LRESULT WINAPI celosia_win32::window::wnd_proc(HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param)
{
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, w_param, l_param))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (w_param == SIZE_MINIMIZED)
            return 0;
        return 0;
    case WM_SYSCOMMAND:
        if ((w_param & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DISPLAYCHANGE: // screen resolution changed
        //ui::variables::monitor::update();
        break;
    case WM_DEVICECHANGE: // monitor changed
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hwnd, msg, w_param, l_param);
}

HWND celosia_win32::window::create(LPCWSTR window_title, ImVec2 pos, ImVec2 size, long window_flags) {
    if (!(variables::windows.find(window_title) == variables::windows.end())) { // window already exists with same title / class
        DestroyWindow(variables::windows[window_title].second);
        UnregisterClassW(window_title, 0);
    }

    WNDCLASSEXW wc = {
        sizeof(wc),
        CS_HREDRAW | CS_VREDRAW,
        wnd_proc,
        0L, 0L,
        GetModuleHandle(nullptr),
        nullptr, nullptr, nullptr, nullptr,
        window_title,
        nullptr
    };

    RegisterClassExW(&wc);

    const HWND hwnd = CreateWindowExW(
        window_flags /*| WS_EX_TRANSPARENT*/,
        wc.lpszClassName,
        window_title,
        WS_POPUP, // WS_OVERLAPPED
        (int)pos.x, (int)pos.y, (int)size.x, (int)size.y,
        nullptr, nullptr,
        wc.hInstance,
        nullptr
    );

    variables::windows[window_title] = { wc, hwnd };
    return hwnd;
}

void celosia_win32::window::show(HWND hwnd) {
    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);
}

void celosia_win32::window::move(HWND hwnd, ImVec2 pos) {
    SetWindowPos(hwnd, 0, (int)pos.x, (int)pos.y, (int)celosia::ui::size.x, (int)celosia::ui::size.y, 0);
}

void celosia_win32::window::destroy() { // every window made through create(), along with its class
    for (auto const& [title, window] : variables::windows) {
        DestroyWindow(window.second); // fails harmlessly if the window was already closed
        UnregisterClassW(window.first.lpszClassName, window.first.hInstance);
    }
    variables::windows.clear();
}
