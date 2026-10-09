#include "../win32/ui_win32.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param);

namespace celosia_win32::window {
    LRESULT WINAPI wnd_proc(HWND hwnd, UINT msg, WPARAM w_param, LPARAM l_param)
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

    HWND create(LPCWSTR window_title, ImVec2 pos, ImVec2 size, long window_flags) {
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
            pos.x, pos.y, size.x, size.y,
            nullptr, nullptr,
            wc.hInstance,
            nullptr
        );

        variables::windows[window_title] = { wc, hwnd };
        return hwnd;
    }

    void show(HWND hwnd) {
        ShowWindow(hwnd, SW_SHOWDEFAULT);
        UpdateWindow(hwnd);
    }
    
    void hide(HWND hwnd) {
        ShowWindow(hwnd, SW_HIDE);
        UpdateWindow(hwnd);
    }

    void move(HWND hwnd, ImVec2 pos) {
        SetWindowPos(hwnd, 0, pos.x, pos.y, celosia::ui::size.x, celosia::ui::size.y, 0);
    }

    void resize(HWND hwnd, ImVec2 size) { // ctodo: WARNING these will become invalid, the user can resize the window
        SetWindowPos(hwnd, 0, 0, 0, size.x, size.y, 0);
    }

    void destroy() { // every window made through create(), along with its class
        for (auto const& [title, window] : variables::windows) {
            DestroyWindow(window.second); // fails harmlessly if the window was already closed
            UnregisterClassW(window.first.lpszClassName, window.first.hInstance);
        }
        variables::windows.clear();
    }
}