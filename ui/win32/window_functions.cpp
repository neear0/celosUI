#include "../win32/ui_win32.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param);

LRESULT WINAPI celosia_win32::window::wnd_proc(HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param)
{
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, w_param, l_param))
        return true;

    switch (msg)
    {
    case WM_SYSCOMMAND:
        if ((w_param & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hwnd, msg, w_param, l_param);
}

HWND celosia_win32::window::create(LPCWSTR window_title, ImVec2 pos, ImVec2 size, long window_flags) {
    if (const auto existing = variables::windows.find(window_title); existing != variables::windows.end()) { // one with the same title / class is replaced
        DestroyWindow(existing->second.second);
        UnregisterClassW(window_title, existing->second.first.hInstance);
        variables::windows.erase(existing);
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

    if (!RegisterClassExW(&wc))
        return nullptr;

    const HWND hwnd = CreateWindowExW(
        window_flags,
        wc.lpszClassName,
        window_title,
        WS_POPUP,
        (int)pos.x, (int)pos.y, (int)size.x, (int)size.y,
        nullptr, nullptr,
        wc.hInstance,
        nullptr
    );
    if (!hwnd) {
        UnregisterClassW(window_title, wc.hInstance);
        return nullptr;
    }

    variables::windows[window_title] = { wc, hwnd };
    return hwnd;
}

void celosia_win32::window::show(HWND hwnd) {
    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);
}

void celosia_win32::window::move(HWND hwnd, ImVec2 pos) {
    SetWindowPos(hwnd, nullptr, (int)pos.x, (int)pos.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE); // only moves it
}

void celosia_win32::window::destroy() { // every window made through create(), along with its class
    for (auto const& [title, window] : variables::windows) {
        DestroyWindow(window.second); // fails harmlessly if the window was already closed
        UnregisterClassW(title.c_str(), window.first.hInstance);
    }
    variables::windows.clear();
}
